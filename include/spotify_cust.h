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

#include "spotify_data.h"

extern "C" {
	#include <libspotify/api.h>
	#include "audio.h"
}


//Forward declaration
class XplodifySession;


// This baby here, the XplodifyHandler, was originally conceived as a singleton. The 
// main idea behind that decision was the fact that we are only supposed to have a 
// single audio queue. Although perhaps fundamentally correct, because singleton's are
// very often a bad decision which may bring nast consequences eventually, I rather leave 
// that decision to the coder - if he decides to to implement several  XplodifyHandler, 
// instances.... then he will be responsible for his actions ;)
//
// The Handler manages incoming thrift requests and most of the logic.
// If we've got several ongoing spotify sessions for different users, the idea is to 
// randomly play music from each of their 'selected' playlists. 
//
// For example: if Joe is listening to his Hip Hop playist, and Jane is listening
// to her oldies playlists. We *do not* want several instances of the XplodifyHandler
// created. We don't need them either. The Handler will randomly select tracks from
// each of the users registered. The Audio Queue must be a singleton, and so we 
// can make the entire handler singleton. One instance is enough to handler all
// incoming requests.
//
class XplodifyHandler 
        : virtual public SpotifyIf
        , public Runnable
        , private Lockable {
    public:
        XplodifyHandler(bool multisession=false);
        void loginSession(SpotifyCredential& _return, const SpotifyCredential& cred);
        bool isLoggedIn(const SpotifyCredential& cred);
        int64_t getStateTS(const SpotifyCredential& cred);
        int64_t getSessionStateTS(const SpotifyCredential& cred);
        void logoutSession(const SpotifyCredential& cred);
        void sendCommand(const SpotifyCredential& cred, const SpotifyCmd::type cmd);
        void switch_session();

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
        void whats_playing(SpotifyTrack& _return);

        void notify_main_thread(void);
        void set_playback_done(bool done);
        int  music_playback(const sp_audioformat * format, 
                const void * frames, int num_frames);
        void audio_fifo_stats(sp_audio_buffer_stats *stats);
        void audio_fifo_flush_now(void);
        void update_timestamp(void);
        std::string get_cachedir();


    protected:
        //implementing runnable
        void run();
    private:
        struct sess_map_entry {
            std::string _uuid;
            std::uintptr_t _sessintptr;
            int _ptrlow32;

            boost::shared_ptr<XplodifySession> session;

            sess_map_entry( const std::string &uuid, uintptr_t sintptr
                    , boost::shared_ptr<XplodifySession> sess ) 
                : _uuid(uuid)
                , _sessintptr(sintptr)
                  //lower 32 bits for greater entropy.
                , _ptrlow32( static_cast<int>(_sessintptr))
                , session(sess)
            {
                return;
            }
            sess_map_entry( const std::string &uuid, const sp_session * sp
                    , boost::shared_ptr<XplodifySession> sess ) 
                : _uuid(uuid)
                , _sessintptr(reinterpret_cast<std::uintptr_t>(sp))
                , session(sess)
            {
            }
        };

        //maybe tagging would make code more readable, but I'm not a fan. Leaving out for now.
        typedef boost::multi_index_container<
            sess_map_entry,
            boost::multi_index::indexed_by<
                boost::multi_index::sequenced<>,
                boost::multi_index::hashed_unique< 
                    BOOST_MULTI_INDEX_MEMBER(sess_map_entry, std::string, _uuid) >,
                boost::multi_index::hashed_unique< 
                    BOOST_MULTI_INDEX_MEMBER(sess_map_entry, std::uintptr_t, _sessintptr) >
            > > sess_map;

        typedef sess_map::nth_index<0>::type sess_map_sequenced;
        typedef sess_map::nth_index<1>::type sess_map_by_uuid;
        typedef sess_map::nth_index<2>::type sess_map_by_sessptr;
        sess_map m_session_cache;

        size_t get_cache_size() {
            sess_map_sequenced& smap = m_session_cache.get<0>();

            return smap.size();
        }

        //no transfer of ownership, we're good with raw pointers.
        typedef std::map<std::string, boost::asio::deadline_timer *> timer_map;
        timer_map m_timers;
        boost::asio::io_service m_io;
        const size_t LOGIN_TO;


        void remove_from_cache(const std::string& uuid);
        void login_timeout(const boost::system::error_code&,
                std::string uuid);
        boost::shared_ptr<XplodifySession> get_session(const std::string& uuid);
        boost::shared_ptr<XplodifySession> get_session(const sp_session * sps);
        boost::shared_ptr<XplodifySession> getActiveSession(void);
        void setActiveSession(boost::shared_ptr<XplodifySession> session);


        //we also need to be able to search by sp_session, that's quite important; 
        //callbacks rely very heavily on it.

        sp_playlistcontainer *                  getPlaylistContainer(SpotifyCredential& cred);
        audio_fifo_t *                          audio_fifo();

        //libspotify wrapped
        audio_fifo_t                            m_audiofifo;

        //proper members
        sess_map_sequenced::iterator            m_sess_it;

        boost::shared_ptr<XplodifySession>      m_active_session;

        int                                     m_playback_done;
        int                                     m_notify_events;
        std::string                             m_sp_cachedir;
        std::time_t                             m_ts;
        const bool                              m_multi;

        //SILENCE NUM SAMPLES THRESHOLD
        enum { SILENCE_N_SAMPLES = 8192 };

};
#endif
