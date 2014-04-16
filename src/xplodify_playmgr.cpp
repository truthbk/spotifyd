#include "xplodify_playmgr.h"

XplodifyPlaybackManager::XplodifyPlaybackManager(
        std::string host, int32_t base_port, uint8_t nprocs) 
    : Lockable()
    , Runnable()
    , m_work(false)
    , m_host(host)
    , m_base_port(base_port)
    , m_nprocs(nprocs)
    , m_play(false)
    , m_master(0)
    , m_cli_it(m_clients.begin())
    , m_user_it(m_users.begin())
{
    for(int i=0 ; i<m_nprocs ; i++) {
        boost::shared_ptr<XplodifyClient> client_ptr(
                new XplodifyClient(m_host, base_port+i));
        m_clients[base_port+i] = client_ptr; 
    }
    return;
}


bool XplodifyPlaybackManager::switch_roles(void){
    //TODO
    uint8_t slave_port = 0;

    lock();
    if(m_clients.size()<2||!m_users.size()) {
        unlock();
        return false;
    }

    if(!m_master) {
        //there was no master, lets just login first user, and set first client
        //as master.
        m_master = m_cli_it->second->_port;
        login(m_user_it->second._username, m_master);
        m_clients[m_master]->_user = &m_user_it->second;
    } else {
        //logout master before switching
        logout(m_master);
        m_clients[m_master]->_master = false;
        m_clients[m_master]->_user = NULL;
        m_master = m_cli_it->second->_port;
    }
    m_clients[m_master]->_master = true;

    //next client is current slave.
    if(m_cli_it++ == m_clients.end()) {
        m_cli_it = m_clients.begin();
    }
    slave_port = m_cli_it->second->_port;

    //next user to log in.
    if(m_user_it++ == m_users.end()){
        m_user_it = m_users.begin();
    }

    //get slave ready.
    login(m_user_it->second._username, slave_port);
    m_clients[slave_port]->_user = &m_user_it->second;

    unlock();
    return true;
}

//Any user we get here is expected to be correct.
void XplodifyPlaybackManager::register_user(std::string user, std::string passwd){
    lock();
    m_users[user] = MgrUser(user, passwd);
    unlock();
}

//Any user we get here is expected to be correct.
bool XplodifyPlaybackManager::login(std::string user, uint8_t client_id){
    //TODO
    SpotifyCredential cred, ret_cred;
    cred._username = m_users[user]._username;
    cred._passwd = m_users[user]._passwd;

    try {
        m_clients[client_id]->_transport->open();
        m_clients[client_id]->_client.check_in(ret_cred, cred);
        m_clients[client_id]->_client.login(ret_cred);
        m_clients[client_id]->_transport->close();
        m_clients[client_id]->_user = &m_users[user];
    } catch (TException &ex) {
        std::cout << ex.what() << "\n"; 
    }
    return true;
}
bool XplodifyPlaybackManager::is_logged_in(std::string user){
    bool logged = m_users[user]._ready;

#if 0
    //TODO
    try {
        m_clients[client_id]._client._transport->open();
        logged = m_clients[client_id]._client.is_logged();
        m_clients[client_id]._client._transport->close();
    } catch (TException &ex) {
        std::cout << ex.what() << "\n"; 
    }
#endif
    return logged;
}
bool XplodifyPlaybackManager::logout(uint8_t client_id) {

    bool logged_out = false;
    //TODO
    try {
        m_clients[client_id]->_transport->open();
        m_clients[client_id]->_client.logout();
        m_clients[client_id]->_transport->close();
        logged_out = true;
    } catch (TException &ex) {
        std::cout << ex.what() << "\n"; 
    }
    return logged_out;
}

bool XplodifyPlaybackManager::logout(std::string user) {
    return false;
}

//call holding lock.
bool XplodifyPlaybackManager::playback_done() {

    bool pback_done = true;
    //only master process plays back.
    if(!m_master) {
        return pback_done;
    }

    try {
        m_clients[m_master]->_transport->open();
        pback_done = m_clients[m_master]->_client.playback_done();
        m_clients[m_master]->_transport->close();
    } catch (TException &ex) {
        std::cout << ex.what() << "\n"; 
    }
    return pback_done;
}


void XplodifyPlaybackManager::select_playlist(
        std::string user, int32_t playlist_id){
    lock();
    m_users[user]._playlist_id = playlist_id;
    unlock();

    return;
}
void XplodifyPlaybackManager::select_playlist(std::string user, std::string playlist){
    lock();
    m_users[user]._playlist = playlist;
    unlock();

    return;
}

//Must be contained in the selected playlist.. else trouble.
void XplodifyPlaybackManager::select_track(std::string user, int32_t track_id){
    lock();
    m_users[user]._track_id = track_id;
    unlock();

    return;
}
void XplodifyPlaybackManager::select_track(std::string user, std::string track){
    lock();
    m_users[user]._track = track;
    unlock();

    return;
}

void XplodifyPlaybackManager::play(void) {
    if(!m_master) {
        return;
    }

    try {
        m_clients[m_master]->_transport->open();
        m_clients[m_master]->_client.play();
        m_clients[m_master]->_transport->close();
    } catch (TException &ex) {
        std::cout << ex.what() << "\n"; 
    }
    return;
}
void XplodifyPlaybackManager::stop(void) {
    if(!m_master) {
        return;
    }

    try {
        m_clients[m_master]->_transport->open();
        m_clients[m_master]->_client.stop();
        m_clients[m_master]->_transport->close();
    } catch (TException &ex) {
        std::cout << ex.what() << "\n"; 
    }
    return;
}
void XplodifyPlaybackManager::next(void) {
    if(!m_master) {
        return;
    }

    try {
        m_clients[m_master]->_transport->open();
        m_clients[m_master]->_client.stop();
        m_clients[m_master]->_transport->close();
    } catch (TException &ex) {
        std::cout << ex.what() << "\n"; 
    }
    return;
}

void XplodifyPlaybackManager::client_set_track(
        uint8_t client_id, std::string username, bool by_id){
    lock();
    try {
        m_clients[client_id]->_transport->open();
        if(by_id) {
            int32_t track_id = m_users[username]._track_id;
            m_clients[client_id]->_client.selectTrackById(track_id);
        } else {
            std::string track_name(m_users[username]._track);
            m_clients[client_id]->_client.selectTrack(track_name);
        }
        m_clients[client_id]->_transport->close();
    } catch (TException &ex) {
        std::cout << ex.what() << "\n"; 
    }
    unlock();
}

void XplodifyPlaybackManager::client_set_playlist(
        uint8_t client_id, std::string username, bool by_id){
    lock();
    try {
        m_clients[client_id]->_transport->open();
        if(by_id) {
            int32_t playlist_id = m_users[username]._playlist_id;
            m_clients[client_id]->_client.selectPlaylistById(playlist_id);
        } else {
            std::string playlist(m_users[username]._playlist);
            m_clients[client_id]->_client.selectPlaylist(playlist);
        }
        m_clients[client_id]->_transport->close();
    } catch (TException &ex) {
        std::cout << ex.what() << "\n"; 
    }
    unlock();
}

void XplodifyPlaybackManager::run() {
    struct timespec ts;


    while(!m_done){
#if _POSIX_TIMERS > 0
        clock_gettime(CLOCK_REALTIME, &ts);
#else
        struct timeval tv;
        gettimeofday(&tv, NULL);
        TIMEVAL_TO_TIMESPEC(&tv, &ts);
#endif
        ts.tv_sec += ipc_poll_to;
        lock();
        if(!m_work) {
            cond_timedwait(&ts);
        }

        if(m_work) {
            //do whatever needs to be done
            ;;
        }

        //check if playback done.
        if(m_master && playback_done()) {
            //TODO: logic is more complex... real simple skeleton for now.
            switch_roles();
            play();
        }

    }
}

//Call holding lock.
boost::shared_ptr<XplodifyPlaybackManager::XplodifyClient> 
XplodifyPlaybackManager::get_client(uint32_t port) {
        client_map::const_iterator cit = m_clients.find(m_master);
        boost::shared_ptr<XplodifyClient> cli(NULL);
        if(cit != m_clients.cend()) {
            cli = cit->second;
        }

        return cli;
}

