#ifndef _SPOTIFY_CUST_HH
#define _SPOTIFY_CUST_HH

#include "SpotifyIPC.h"

class XplodifyIPCServer
    : virtual public SpotifyIPCIf
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

}

#endif //_SPOTIFY_CUST_HH
