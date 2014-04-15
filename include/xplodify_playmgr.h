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
#include "runnable.h"
#include "SpotifyIPC.h"

//Forward declaration.
class XplodifyMultiHandler;

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;

namespace {
    const uint8_t ipc_poll_to = 1;
}

class XplodifyPlaybackManager 
    : private Lockable
    , private Runnable
{
    friend class XplodifyMultiHandler;
    public:
        XplodifyPlaybackManager(
                std::string host, int32_t base_port, uint8_t n_procs);
        bool switch_roles(void);
        void register_user(std::string user, std::string passwd);
        bool login(std::string user, uint8_t client_id);
        bool is_logged_in(std::string user);
        bool logout(uint8_t client_id);
        bool logout(std::string user);
        bool playback_done();

        void select_playlist(std::string user, int32_t playlist_id);
        void select_playlist(std::string user, std::string playlist);
        void select_track(std::string user, int32_t track_id);
        void select_track(std::string user, std::string track);

        void play(void);
        void stop(void);
        void next(void);
        //void prev(void);

    protected:
        //implementing runnable
        virtual void run();

    private:
        bool              m_work;
        const std::string m_host;
        const int32_t     m_base_port;
        const uint8_t     m_nprocs;
        bool              m_play;
        uint8_t           m_master; //port for master process

        struct MgrUser {
            std::string     _username;
            std::string     _passwd;
            std::string     _uuid;
            std::string     _playlist;
            int32_t         _playlist_id;
            std::string     _track;
            int32_t         _track_id;
            bool            _ready;

            MgrUser() {
            };
            MgrUser(std::string user, std::string passwd)
                : _username(user)
                , _passwd(passwd)
                , _uuid("")
                , _playlist("")
                , _playlist_id(0)
                , _track("")
                , _track_id(0)
                , _ready(false)
            {
            };
        };
        struct XplodifyClient {
            std::string _host;
            uint32_t _port;
            MgrUser * _user;

            boost::shared_ptr<TSocket> _socket;
            boost::shared_ptr<TBufferedTransport> _transport;
            boost::shared_ptr<TBinaryProtocol> _protocol;

            SpotifyIPCClient _client;

            bool _master;

            XplodifyClient(std::string host, int32_t port)
                : _host(host)
                , _port(port)
                , _user(NULL)
                , _socket(new TSocket(_host, _port))
                , _transport(new TBufferedTransport(_socket))
                , _protocol(new TBinaryProtocol(_transport))
                , _client(_protocol)
                , _master(false)
            {
            };
        };

        boost::shared_ptr<XplodifyClient> get_client(uint32_t port);

        typedef std::map<uint32_t, boost::shared_ptr<XplodifyClient> > client_map;
        typedef std::map<std::string, MgrUser> user_map;

        client_map m_clients;
        user_map m_users;
        client_map::iterator                   m_cli_it;
        user_map::iterator                     m_user_it;
};


#endif //_XPLODIFY_PLMGR_HH
