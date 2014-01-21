#include <protocol/TBinaryProtocol.h>
#include <server/TSimpleServer.h>
#include <transport/TServerSocket.h>
#include <transport/TBufferTransports.h>

//Boost.
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include <iostream>
#include <cstring>
#include <cerrno>
#include <string>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "SpotifyIPC.h"

#include "xplodify_sess.h"
#include "xplodify_plc.h"
#include "xplodify_pl.h"
#include "spotify_ipc_srv.h"

extern "C" {
    #include <libspotify/api.h>
    #include "audio.h"
}


XplodifyIPCServer::XplodifyIPCServer()
    : Runnable()
    , Lockable()
    , m_sh(std::to_string(getpid()))
    , m_ts(std::time(NULL))
    , m_master(false)
    , m_logging_in(false)
    , LOGIN_IPC_TO(SP_IPC_TIMEOUT)
    , m_timer(m_io)
{
    m_sh.start();
}



bool XplodifyIPCServer::set_master() {
    //TODO: method body
    m_master = true;
    return m_master;
}
bool XplodifyIPCServer::set_slave() {
    //TODO: method body
    m_master = false;
    return m_master;
}

void XplodifyIPCServer::check_in(SpotifyIPCCredential& _return, const SpotifyIPCCredential& cred) {

    if(!m_uuid.empty()) {
        _return.__set__uuid(std::string("")); //Empty string is a failure to check in.
        return;
    }
    m_uuid = std::string(m_sh.check_in());
    _return = cred;
    _return.__set__uuid(m_uuid); //Empty string is a failure to check in.
}

bool XplodifyIPCServer::check_out() {
    bool res = false;
    res =  m_sh.check_out(m_uuid);
    if(res) {
        m_uuid.clear();
    }
    return res;
}

bool XplodifyIPCServer::login(const SpotifyIPCCredential& cred) {

    if(cred._uuid != m_uuid) {
        return false;
    }
    m_logging_in = m_sh.login(m_uuid, cred._username, cred._passwd);

    if(m_logging_in) {
        m_timer.expires_from_now(boost::posix_time::seconds(LOGIN_IPC_TO));
        m_timer.async_wait(boost::bind(&XplodifyIPCServer::login_timeout,
                    this, boost::asio::placeholders::error, m_uuid));
    }

    return m_logging_in;
}
void XplodifyIPCServer::logout() {
    m_sh.logout(m_uuid);
    return;
}

bool XplodifyIPCServer::is_logged(){
    bool status = m_sh.login_status(m_uuid);
    return status;
}
void XplodifyIPCServer::selectPlaylist(const std::string& playlist) {

    bool status = m_sh.select_playlist(m_uuid, playlist);
    return;
}
void XplodifyIPCServer::selectPlaylistById(const int32_t plist_id) {

    bool status = m_sh.select_playlist(m_uuid, plist_id);
    return;
}
void XplodifyIPCServer::selectTrack(const std::string& track){

    bool status = m_sh.select_track(m_uuid, track);
    return;
}
void XplodifyIPCServer::selectTrackById(const int32_t track_id){

    bool status = m_sh.select_track(m_uuid, track_id);
    return;
}

void XplodifyIPCServer::play(){

    m_sh.play();
    return;
}
void XplodifyIPCServer::stop(){

    m_sh.stop();
    return;
}
void XplodifyIPCServer::terminate_proc() {
    lock();
    m_done = true;
    unlock();
    return;
}



void XplodifyIPCServer::update_timestamp(void) {
    lock();
    m_ts = std::time(NULL);
    unlock();
}


void XplodifyIPCServer::run() {

    while(!m_done){
        m_io.poll();
        m_io.reset();
        //yield CPU
        sleep(0);
    }
}


void XplodifyIPCServer::login_timeout(const boost::system::error_code& e, 
        std::string uuid) {

    if(uuid != m_uuid) {
        return;
    }

    //check session status...
    bool logged = m_sh.login_status(m_uuid);

    if(logged) {
#ifdef _DEBUG
        std::cout << "Session: " << uuid << " Succesfully logged in ...\n";
#endif
        return;
    }
    m_logging_in = false;

    m_sh.update_timestamp();
}

