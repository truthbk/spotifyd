#ifndef _SPOTIFY_CUST_HH
#define _SPOTIFY_CUST_HH

#include <cstdint>
#include <ctime>
#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "Spotify.h"

#include "lockable.h"
#include "runnable.h"
#include "xplodify_handler.h"

#include "spotify_data.h"

extern "C" {
	#include <libspotify/api.h>
	#include "audio.h"
}


//Forward declaration
class XplodifySession;


// This baby here, the XplodifyServer, was originally conceived as a singleton. The 
// main idea behind that decision was the fact that we are only supposed to have a 
// single audio queue. Although perhaps fundamentally correct, because singleton's are
// very often a bad decision which may bring nast consequences eventually, I rather leave 
// that decision to the coder - if he decides to to implement several  XplodifyServer, 
// instances.... then he will be responsible for his actions ;)
//
// The Handler manages incoming thrift requests and most of the logic.
// If we've got several ongoing spotify sessions for different users, the idea is to 
// randomly play music from each of their 'selected' playlists. 
//
// For example: if Joe is listening to his Hip Hop playist, and Jane is listening
// to her oldies playlists. We *do not* want several instances of the XplodifyServer
// created. We don't need them either. The Handler will randomly select tracks from
// each of the users registered. The Audio Queue must be a singleton, and so we 
// can make the entire handler singleton. One instance is enough to handler all
// incoming requests.
//
class XplodifyServer 
        : virtual public SpotifyIf
        , public Runnable
        , private Lockable {
    public:
        XplodifyServer(bool multisession=false);
        bool is_service_ready();
        ::SpotifyMode::type server_mode();
        void check_in(SpotifyCredential& _return, const SpotifyCredential& cred);
        bool check_out(const SpotifyCredential& cred);
        bool loginSession(const SpotifyCredential& cred);
        bool isLoggedIn(const SpotifyCredential& cred);
        int64_t getStateTS(const SpotifyCredential& cred);
        int64_t getSessionStateTS(const SpotifyCredential& cred);
        void logoutSession(const SpotifyCredential& cred);
        void sendCommand(const SpotifyCredential& cred, const SpotifyCmd::type cmd);
        //void switch_session();

        void search(SpotifyPlaylist& _return, const SpotifyCredential& cred,
                const SpotifySearch& criteria);
        void getPlaylists(SpotifyPlaylistList& _return, const SpotifyCredential& cred);
        void getPlaylist(SpotifyPlaylist& _return, const SpotifyCredential& cred,
                const int32_t plist_id);
        void getPlaylistByName(SpotifyPlaylist& _return, const SpotifyCredential& cred,
                const std::string& name);
        void selectPlaylist(const SpotifyCredential& cred, const std::string& playlist);
        void selectPlaylistById(const SpotifyCredential& cred, const int32_t plist_id);
        void selectTrack(const SpotifyCredential& cred, const std::string& track);
        void selectTrackById(const SpotifyCredential& cred, const int32_t track_id);
        bool merge2playlist(const SpotifyCredential& cred, const std::string& pl,
                const SpotifyPlaylist& tracks);
        bool add2playlist(const SpotifyCredential& cred, const std::string& pl,
                const SpotifyTrack& track);
        void whats_playing(SpotifyTrack& _return, const SpotifyCredential& cred );

    protected:
        //runnable, to process timer events...
        void run();

    private:
        void update_timestamp(void);
        void login_timeout(const boost::system::error_code&,
                std::string uuid);
        void cacheload_timeout(const boost::system::error_code&,
                std::string uuid);


        boost::shared_ptr<XplodifyHandler> m_sh;

        //Session login timers...
        //no transfer of ownership, we're good with raw pointers.
        typedef std::map<std::string, boost::asio::deadline_timer *> timer_map;
        timer_map m_timers;

        std::time_t                             m_ts;
        boost::asio::io_service                 m_io;

        const bool m_multi;

};
#endif
