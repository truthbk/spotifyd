#include "xplodify_playmgr.h"

XplodifyPlaybackManager::XplodifyPlaybackManager(
        std::string host, int32_t base_port, uint8_t nprocs) 
    : Lockable()
    , Runnable()
    , m_host(host)
    , m_base_port(base_port)
    , m_nprocs(nprocs)
    , m_play(false)
    , m_master(0)
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
    try {
    } catch (TException &ex) {
        std::cout << ex.what() << "\n"; 
    }
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
    SpotifyIPCCredential cred, ret_cred;
    cred._username = m_users[user]._username;
    cred._passwd = m_users[user]._passwd;

    try {
        m_clients[client_id]->_transport->open();
        m_clients[client_id]->_client.check_in(ret_cred, cred);
        m_clients[client_id]->_client.login(ret_cred);
        m_clients[client_id]->_transport->close();
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
bool XplodifyPlaybackManager::logout(std::string user) {

    bool logged_out = false;
    //TODO
#if 0
    try {
        m_clients[client_id]._client._transport->open();
        logged_out = m_clients[client_id]._client.logout();
        m_clients[client_id]._client._transport->close();
    } catch (TException &ex) {
        std::cout << ex.what() << "\n"; 
    }
#endif
    return logged_out;
}

void XplodifyPlaybackManager::select_playlist(
        std::string user, int32_t playlist_id){
    //TODO
    lock();
    m_users[user]._playlist_id = playlist_id;

    client_map::iterator it;
    for( it = m_clients.begin() ; it != m_clients.end() ; it++){
        try {
            it->second->_transport->open();
            it->second->_client.selectPlaylistById(playlist_id);
            it->second->_transport->close();
        } catch (TException &ex) {
            std::cout << ex.what() << "\n"; 
        }
    }
    unlock();
    return;
}
void XplodifyPlaybackManager::select_playlist(std::string user, std::string playlist){
    //TODO
    lock();
    m_users[user]._playlist = playlist;

    client_map::iterator it;
    for( it = m_clients.begin() ; it != m_clients.end() ; it++){
        try {
            it->second->_transport->open();
            it->second->_client.selectPlaylist(playlist);
            it->second->_transport->close();
        } catch (TException &ex) {
            std::cout << ex.what() << "\n"; 
        }
    }
    unlock();
    return;
}

//Must be contained in the selected playlist.. else trouble.
void XplodifyPlaybackManager::select_track(std::string user, int32_t track_id){
    //TODO
    lock();
    m_users[user]._track_id = track_id;

    client_map::iterator it;
    for( it = m_clients.begin() ; it != m_clients.end() ; it++){
        //propagate selection 
        try {
            it->second->_transport->open();
            it->second->_client.selectTrackById(track_id);
            it->second->_transport->close();
        } catch (TException &ex) {
            std::cout << ex.what() << "\n"; 
        }
    }
    unlock();
    return;
}
void XplodifyPlaybackManager::select_track(std::string user, std::string track){
    //TODO
    lock();
    m_users[user]._track = track;

    client_map::iterator it;
    for( it = m_clients.begin() ; it != m_clients.end() ; it++){
        //propagate selection 
        try {
            it->second->_transport->open();
            it->second->_client.selectTrack(track);
            it->second->_transport->close();
        } catch (TException &ex) {
            std::cout << ex.what() << "\n"; 
        }
    }
    unlock();
    return;
}

void XplodifyPlaybackManager::play(void) {
    //TODO
#if 0
    try {
        m_clients[client_id]._client._transport->open();
    //TODO
        m_clients[client_id]._client._transport->close();
    } catch (TException &ex) {
        std::cout << ex.what() << "\n"; 
    }
#endif
    return;
}
void XplodifyPlaybackManager::stop(void) {
    //TODO
#if 0
    try {
        m_clients[client_id]._client._transport->open();
    //TODO
        m_clients[client_id]._client._transport->close();
    } catch (TException &ex) {
        std::cout << ex.what() << "\n"; 
    }
#endif
    return;
}


void XplodifyPlaybackManager::run() {
    while(!m_done){
        sleep(1);
    }
}
