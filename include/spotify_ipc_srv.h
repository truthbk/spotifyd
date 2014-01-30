#ifndef _SPOTIFY_IPC_SRV_HH
#define _SPOTIFY_IPC_SRV_HH

//Boost.
#include <boost/shared_ptr.hpp>

#include "lockable.h"
#include "runnable.h"
#include "xplodify_handler.h"

#include "SpotifyIPC.h"


class XplodifyIPCServer
    : virtual public SpotifyIPCIf
    , public Runnable
    , public Lockable { 

    public:
        XplodifyIPCServer();

        bool set_master();
        bool set_slave();
        void check_in(SpotifyCredential& _return, const SpotifyCredential& cred);
        bool check_out();
        bool login(const SpotifyCredential& cred);
        void logout();
        bool is_logged();
        void selectPlaylist(const std::string& playlist);
        void selectPlaylistById(const int32_t plist_id);
        void selectTrack(const std::string& track);
        void selectTrackById(const int32_t track_id);
        void play();
        void stop();
        void terminate_proc();


    protected:
        //implementing runnable
        void run();

    private:
        void login_timeout(const boost::system::error_code&, std::string uuid);
        void update_timestamp(void);

        XplodifyHandler                         m_sh;
        std::time_t                             m_ts;
        bool                                    m_master;
        bool                                    m_logging_in;

        const size_t                            LOGIN_IPC_TO;
        boost::asio::io_service                 m_io;
        boost::asio::deadline_timer             m_timer;

        //IPC server meant to only take a single session.
        //We enforce that here(?)
        std::string                             m_uuid; 

};

#endif //_SPOTIFY_IPC_SRV_HH
