#ifndef _XPLODIFY_PLMGR_HH
#define _XPLODIFY_PLMGR_HH

#include "Spotify.h"


class XplodifyPlaybackManager 
    : private Lockable
{
    public:
        XplodifyPlaybackManager(uint8_t n_procs, uint32_t base_port);
        bool switch_roles(void);
        bool login(std::string user, std::string passwd);
        bool is_logged_in(std::string user);
        bool logout(std::string user);

        void select_playlist(std::string user, int32_t playlist_id);
        void select_playlist(std::string user, std::string playlist);
        void select_track(std::string user, int32_t track_id);
        void select_track(std::string user, std::string track);

        void play(void);
        void stop(void);

    private:
        const uint8_t n_procs;
        const uint32_t base_port;
        uint8_t master;

        struct XplodifyClient {
            public:
                boost::shared_ptr<TSocket> socket;
                boost::shared_ptr<TBufferedTransport> transport;
                boost::shared_ptr<TBinaryProtocol> protocol;

                SpotifyClient client;
                bool master;
        }

        std::map<uint32_t,boost::shared_ptr<XplodifyClient> > clients;
        std::map<std::string, boost::shared_ptr<XplodifyClient> > users;
}


#endif //_XPLODIFY_PLMGR_HH
