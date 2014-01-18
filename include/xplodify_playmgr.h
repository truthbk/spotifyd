#ifndef _XPLODIFY_PLMGR_HH
#define _XPLODIFY_PLMGR_HH

#include <cstdint>
#include <map>
#include <string>

#include <boost/shared_ptr.hpp>

#include "lockable.h"
#include "Spotify.h"


class XplodifyPlaybackManager 
    : private Lockable
{
    public:
        XplodifyPlaybackManager(
                std::string host, uint32_t base_port, uint8_t n_procs);
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
        const uint32_t m_host;
        const uint32_t m_base_port;
        const uint8_t m_n_procs;
        uint8_t master;

        struct XplodifyClient {
            std::string _host;
            uint32_t _port;

            boost::shared_ptr<TSocket> _socket;
            boost::shared_ptr<TBufferedTransport> _transport;
            boost::shared_ptr<TBinaryProtocol> _protocol;

            SpotifyClient _client;

            bool _master;

            XplodifyClient(std::string host, uint32_t port)
                : _host(host)
                , _port(port)
                , _socket(new TSocket(_host, _port))
                , _transport(new TBufferedTransport(_socket))
                , _protocol(new TBinaryProtocol(_transport))
                , _client(_protocol)
                , _master(false)
            {
            }
        }

        std::map<uint32_t,boost::shared_ptr<XplodifyClient> > clients;
        std::map<std::string, boost::shared_ptr<XplodifyClient> > users;
}


#endif //_XPLODIFY_PLMGR_HH
