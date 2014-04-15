#include <string>

#include "xplodify_multi_handler.h"
#include "spotify_data.h"

using namespace xplodify::constants;

XplodifyMultiHandler::XplodifyMultiHandler()
    : XplodifyHandler()
    , m_playmgr(
            std::string("localhost"),
            IPCSRV_BASE_PORT,
            IPC_DEF_NPROCS )
{
    //get the playback manager running
    m_playmgr.start();
}

XplodifyMultiHandler::XplodifyMultiHandler(
        std::string host,
        uint32_t port,
        uint8_t nprocs,
        std::string cachedir)
    : XplodifyHandler(cachedir)
    , m_playmgr(host, port, nprocs)
{
    //get the playback manager running
    m_playmgr.start();
}


void XplodifyMultiHandler::register_playback(std::string uuid) {
    user_map_by_uuid& user_by_uuid = m_user_cache.get<1>();

    user_map_by_uuid::iterator sit = user_by_uuid.find(uuid);
    if( sit == m_user_cache.get<1>().end() ) {
        return;
    }

    m_playmgr.register_user(sit->_user,sit->_passwd);
    return;
}

bool XplodifyMultiHandler::select_playlist(std::string uuid, int pid) {
    //TODO
    return false;
}
bool XplodifyMultiHandler::select_playlist(std::string uuid, std::string pname) {
    //TODO
    return false;
}
bool XplodifyMultiHandler::select_track(std::string uuid, int tid) {
    //TODO
    return false;
}
bool XplodifyMultiHandler::select_track(std::string uuid, std::string tname) {
    //TODO
    return false;
}
boost::shared_ptr<XplodifyTrack> XplodifyMultiHandler::whats_playing(void) {
    return boost::shared_ptr<XplodifyTrack>();
}
boost::shared_ptr<XplodifyTrack> XplodifyMultiHandler::whats_playing(std::string uuid) {
    return boost::shared_ptr<XplodifyTrack>();
}
void XplodifyMultiHandler::play() {
    //TODO: only admin can play/stop/etc
    m_playmgr.play();
}
void XplodifyMultiHandler::stop() {
    //TODO: only admin can play/stop/etc
    m_playmgr.stop();
}
void XplodifyMultiHandler::next(){
    //TODO: only admin can play/stop/etc
    m_playmgr.next();
}
void XplodifyMultiHandler::prev() {
    //TODO: only admin can play/stop/etc
    //m_playmgr.prev();
    return;
}
void XplodifyMultiHandler::set_playback_mode(SpotifyCmd::type cmd) {
    return;
}
void XplodifyMultiHandler::set_repeat_mode(SpotifyCmd::type cmd) {
    return;
}
