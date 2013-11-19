#ifndef _SPOTIFY_CUST_HH
#define _SPOTIFY_CUST_HH

//Boost.
#include <boost/shared_ptr.hpp>

#include "SpotifyIPC.h"

extern "C" {
	#include <libspotify/api.h>
	#include "audio.h"
}

class XplodifyIPCServer
    : virtual public SpotifyIPCIf
    , public Runnable
    , private Lockable {
{
    public:
        XplodifyIPCServer();

        bool set_master();
        bool set_slave();
        void login(const SpotifyIPCCredential& cred);
        bool logout();
        bool is_logged(const SpotifyIPCCredential& cred);
        void selectPlaylist(const std::string& playlist);
        void selectPlaylistById(const int32_t plist_id);
        void selectTrack(const std::string& track);
        void selectTrackById(const int32_t track_id);
        void play();
        void stop();

    protected:
        //implementing runnable
        void run();

    private:
        boost::shared_ptr<XplodifySession>      m_session;

        int                                     m_playback_done;
        int                                     m_notify_events;
        std::string                             m_sp_cachedir;
        std::time_t                             m_ts;
        const bool                              m_multi;

}

#endif //_SPOTIFY_CUST_HH
