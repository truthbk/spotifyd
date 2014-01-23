#ifndef _XPLODIFY_PLMGR_HH
#define _XPLODIFY_PLMGR_HH

#include <cstdint>
#include <map>
#include <string>

#include <transport/TSocket.h>
#include <protocol/TBinaryProtocol.h>
#include <transport/TBufferTransports.h>

#include <boost/shared_ptr.hpp>

#include "lockable.h"
#include "SpotifyIPC.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;

class XplodifyPlaybackManager 
    : private Lockable
{
    public:
        XplodifyPlaybackManager(
                std::string host, int32_t base_port, uint8_t n_procs);
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
        const std::string m_host;
        const int32_t     m_base_port;
        const uint8_t     m_nprocs;
        uint8_t master;

        struct XplodifyClient {
            std::string _host;
            uint32_t _port;

            boost::shared_ptr<TSocket> _socket;
            boost::shared_ptr<TBufferedTransport> _transport;
            boost::shared_ptr<TBinaryProtocol> _protocol;

            SpotifyIPCClient _client;

            bool _master;

            XplodifyClient(std::string host, int32_t port)
                : _host(host)
                , _port(port)
                , _socket(new TSocket(_host, _port))
                , _transport(new TBufferedTransport(_socket))
                , _protocol(new TBinaryProtocol(_transport))
                , _client(_protocol)
                , _master(false)
            {
            };
        };

        std::map<uint32_t, boost::shared_ptr<XplodifyClient> > clients;
        std::map<std::string, boost::shared_ptr<XplodifyClient> > users;
};


#endif //_XPLODIFY_PLMGR_HH
