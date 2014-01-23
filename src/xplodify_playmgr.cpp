#include "xplodify_playmgr.h"



XplodifyPlaybackManager::XplodifyPlaybackManager(
        std::string host, int32_t base_port, uint8_t nprocs) 
    : m_host(host)
    , m_base_port(base_port)
    , m_nprocs(nprocs)
{
    for(int i=0 ; i<m_nprocs ; i++) {
        boost::shared_ptr<XplodifyClient> client_ptr(
                new XplodifyClient(m_host, base_port+i));
        clients[base_port+i] = client_ptr; 
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
bool XplodifyPlaybackManager::login(std::string user, std::string passwd){
    //TODO
    SpotifyIPCCredential cred, ret_cred;

    cred._username = user;
    cred._passwd = passwd;
    try {
        users[user]->_transport->open();
        users[user]->_client.check_in(ret_cred, cred);
        users[user]->_client.login(ret_cred);
        users[user]->_transport->close();
    } catch (TException &ex) {
        std::cout << ex.what() << "\n"; 
    }
    return true;
}
bool XplodifyPlaybackManager::is_logged_in(std::string user){
    //TODO
    bool logged = false;
    try {
        users[user]->_transport->open();
        logged = users[user]->_client.is_logged();
        users[user]->_transport->close();
    } catch (TException &ex) {
        std::cout << ex.what() << "\n"; 
    }
    return logged;
}
bool XplodifyPlaybackManager::logout(std::string user) {

    bool logged_out = false;
    //TODO
#if 0
    try {
        users[user]->_transport->open();
        logged_out = users[user]->_client.logout();
        users[user]->_transport->close();
    } catch (TException &ex) {
        std::cout << ex.what() << "\n"; 
    }
#endif
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
        std::cout << ex.what() << "\n"; 
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
        std::cout << ex.what() << "\n"; 
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
        std::cout << ex.what() << "\n"; 
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
        std::cout << ex.what() << "\n"; 
    }
    return;
}

void XplodifyPlaybackManager::play(void) {
    //TODO
#if 0
    try {
        users[user]->_transport->open();
    //TODO
        users[user]->_transport->close();
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
        users[user]->_transport->open();
    //TODO
        users[user]->_transport->close();
    } catch (TException &ex) {
        std::cout << ex.what() << "\n"; 
    }
#endif
    return;
}
