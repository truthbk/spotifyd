#include "xplodify_playmgr.h"



XplodifyPlaybackManager::XplodifyPlaybackManager(
        std::string host, uint32_t base_port, uint8_t nprocs) 
    : m_host(host)
    , m_base_port(base_port)
    , m_n_procs(n_procs)
{
    for(int i=0 ; i<n_procs ; i++) {
        boost::shared_ptr<XplodifyClient> client_ptr(
                new XplodifyClient(_host, base_port+i));
        clients[base_port+i] = client_ptr; 
    }
    return;
}


bool XplodifyPlaybackManager::switch_roles(void){
    //TODO
    try {
    } catch (TException &ex) {
        std::cout << tx.what() << "\n"; 
    }
    return;
}
//Any user we get here is expected to be correct.
bool XplodifyPlaybackManager::login(std::string user, std::string passwd){
    //TODO
    SpotifyIPCCredential cred;
    cred._username = user;
    cred._passwd = _uuid;
    try {
        users[user]->_transport->open();
        users[user]->_client.login(
                users[user]->_client.check_in(cred));
        users[user]->_transport->close();
    } catch (TException &ex) {
        std::cout << tx.what() << "\n"; 
    }
    return;
}
bool XplodifyPlaybackManager::is_logged_in(std::string user){
    //TODO
    bool logged = false;
    try {
        users[user]->_transport->open();
        logged = users[user]->_client.is_logged();
        users[user]->_transport->close();
    } catch (TException &ex) {
        std::cout << tx.what() << "\n"; 
    }
    return logged;
}
bool XplodifyPlaybackManager::logout(std::string user) {
    //TODO
    bool logged_out = false;
    try {
        users[user]->_transport->open();
        logged_out = users[user]->_client.logout();
        users[user]->_transport->close();
    } catch (TException &ex) {
        std::cout << tx.what() << "\n"; 
    }
    return logged_out;
}

void XplodifyPlaybackManager::select_playlist(
        std::string user, int32_t playlist_id){
    //TODO
    try {
        users[user]->_transport->open();
        users[user]->_client.selectPlaylistById(playlist_id);
        users[user]->_transport->close();
    } catch (TException &ex) {
        std::cout << tx.what() << "\n"; 
    }
    return;
}
void XplodifyPlaybackManager::select_playlist(std::string user, std::string playlist){
    //TODO
    try {
        users[user]->_transport->open();
        users[user]->_client.selectPlaylist(playlist);
        users[user]->_transport->close();
    } catch (TException &ex) {
        std::cout << tx.what() << "\n"; 
    }
    return;
}

//Must be contained in the selected playlist... else trouble.
void XplodifyPlaybackManager::select_track(std::string user, int32_t track_id){
    //TODO
    try {
        users[user]->_transport->open();
        users[user]->_client.selectTrackById(track_id);
        users[user]->_transport->close();
    } catch (TException &ex) {
        std::cout << tx.what() << "\n"; 
    }
    return;
}
void XplodifyPlaybackManager::select_track(std::string user, std::string track){
    //TODO
    try {
        users[user]->_transport->open();
        users[user]->_client.selectTrack(track);
        users[user]->_transport->close();
    } catch (TException &ex) {
        std::cout << tx.what() << "\n"; 
    }
    return;
}

void XplodifyPlaybackManager::play(void) {
    try {
        users[user]->_transport->open();
    //TODO
        users[user]->_transport->close();
    } catch (TException &ex) {
        std::cout << tx.what() << "\n"; 
    }
    return;
}
void XplodifyPlaybackManager::stop(void) {
    //TODO
    try {
        users[user]->_transport->open();
    //TODO
        users[user]->_transport->close();
    } catch (TException &ex) {
        std::cout << tx.what() << "\n"; 
    }
    return;
}
