//Boost.
#include <boost/algorithm/string.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <iostream>
#include <cstring>
#include <cerrno>
#include <string>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "Spotify.h"

#include "xplodify_sess.h"
#include "xplodify_plc.h"
#include "xplodify_pl.h"
#include "spotify_cust.h"


 std::string XplodifyHandler::login(const std::string& username, const::string& passwd){
     if(!m_active_session) {
        m_active_session = XplodifySession::create(this);
        if(m_active_session->init_session(g_appkey, g_appkey_size )) {
#ifdef _DEBUG
            std::cout << "Unexpected error creating session. "<< std::endl;
#endif
            sess.reset();
            return;
        }
     }
     if(m_active_session->is_logged_in()) {
         return m_active_session->get_uuid();
     }


     m_active_session->login(cred._username, cred._passwd);


     lock();
     //generate UUID
     const boost::uuids::uuid uuid = boost::uuids::random_generator()();
     const std::string uuid_str = boost::lexical_cast<std::string>(uuid);

     m_active_session->set_uuid(uuid_str);
     update_timestamp();
     unlock();

     return uuid_str;
 }

 std::string XplodifyHandler::login(const std::string& token){
     //TODO
 }
 bool XplodifyHandler::login_status(std::string uuid){
     //TODO
 }
 std::string XplodifyHandler::logout(std::string uuid){
 }
 std::vector< std::string > XplodifyHandler::get_playlists(string uuid){
 }
 std::vector< std::string > XplodifyHandler::get_tracks(string uuid, int pid){
 }
 bool XplodifyHandler::select_playlist(std::string uuid, int pid){
 }
 bool XplodifyHandler::select_playlist(std::string uuid, std::string pname){
 }
 bool XplodifyHandler::select_track(std::string uuid, int tid){
 }
 bool XplodifyHandler::select_track(std::string uuid, std::string tname){
 }
 void XplodifyHandler::play(){
 }
 void XplodifyHandler::stop(){
 }

 void XplodifyHandler::notify_main_thread(void){
 }
 void XplodifyHandler::set_playback_done(bool done){
 }
 int  XplodifyHandler::music_playback(const sp_audioformat * format, 
         const void * frames, int num_frames) {
 }
 void XplodifyHandler::audio_fifo_stats(sp_audio_buffer_stats *stats){
 }
 void XplodifyHandler::audio_fifo_flush_now(void){
 }

 int64_t XplodifyHandler::get_session_state(std::string uuid){
 }
 void XplodifyHandler::update_timestamp(void){
 }
 std::string XplodifyHandler::get_cachedir(){
 }

 void XplodifyHandler::run(){
 }
