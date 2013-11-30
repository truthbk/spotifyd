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

std::string XplodifyHandler::check_in(){
    boost::shared_ptr< XplodifySession > sess = XplodifySession::create(this);
    if(sess->init_session(g_appkey, g_appkey_size )) {
#ifdef _DEBUG
        std::cout << "Unexpected error creating session. "<< std::endl;
#endif
        sess.reset();
        return std::string(); //empty string if error.
    }

    m_session_cache.get<1>().insert(sess_map_entry( uuid_str, 
                const_cast<const sp_session *>(sess->get_session()), sess ));

    lock();
    //generate UUID
    const boost::uuids::uuid uuid = boost::uuids::random_generator()();
    const std::string uuid_str = boost::lexical_cast<std::string>(uuid);

    m_active_session->set_uuid(uuid_str);
    update_timestamp();
    unlock();

    return uuid_str;
}

bool XplodifyHandler::check_out(const std::string& uuid){
    boost::shared_ptr< XplodifySession > sess = get_session(uuid);
    if(!sess) {
        return false;
    }
    if(sess == m_active_session) {
        logout(uuid);
    }
#if 0
    if(err == SP_ERROR_OK )
    {
        remove_from_cache(cred._uuid);
    }
#endif
}

//Returning true means login process has been initiated, not succesful.
bool XplodifyHandler::login(const std::string& uuid,
        const std::string& username, const::string& passwd){

    boost::shared_ptr< XplodifySession > sess = get_session(uuid);
    if(!sess) {
        //user not checked in.
        return false;
    }

    if(sess->is_logged_in()) {
        return true;
    }


    sess->login(cred._username, cred._passwd);
    return true;

}

std::string XplodifyHandler::login(const std::string& uuid, const std::string& token){

    boost::shared_ptr< XplodifySession > sess = get_session(uuid);
    if(!sess) {
        //user not checked in.
        return false;
    }
    // MORE TODO

    return true;
}
bool XplodifyHandler::login_status(std::string uuid){
    boost::shared_ptr< XplodifySession > 
        sess = get_session(uuid);

    if(!sess)
        return false;

    return sess->get_logged_in();
}

//only the active session can be logged in
bool XplodifyHandler::logout(std::string uuid){
    sp_error err;
    bool switched = false;

    boost::shared_ptr<XplodifySession> sess = get_session(cred._uuid);
    if(!sess) {
        return;
    }

    lock();
    if(sess != m_active_session) 
    {
        //already logged out..
        return true;

    }

    //switch_session();
    sess->flush();
    err = sp_session_logout(sess->get_session());
    sp_session_release(sess->get_session());
    m_active_session.reset();

#ifdef _DEBUG
    std::cout << "XplodifySession use count: " << sess.use_count()  << std::endl;
#endif
    unlock();
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
    int next_timeout = 0;

    while(!m_done)
    {
        if (next_timeout == 0) {
            while(!m_notify_events && !m_playback_done) {
                cond_wait();
            }
        } else {
            struct timespec ts;

#if _POSIX_TIMERS > 0
            clock_gettime(CLOCK_REALTIME, &ts);
#else
            struct timeval tv;
            gettimeofday(&tv, NULL);
            TIMEVAL_TO_TIMESPEC(&tv, &ts);
#endif
            ts.tv_sec += next_timeout / 1000;
            ts.tv_nsec += (next_timeout % 1000) * 1000000;

            cond_timedwait(&ts);
        }

        m_notify_events = 0;
        unlock();

        if (m_playback_done) {
            //track_ended();
            m_playback_done = 0;
        }

        do {
            //we can only do work on the active session!
            lock()
                sp_session_process_events(m_active_session->get_session(), &next_timeout);
            unlock();
        } while (next_timeout == 0);

#if 0
        m_io.poll();
        m_io.reset();
#endif

        lock();
    }
}

//change session, allow it to be played.
void XplodifyHandler::switch_session() {
    sp_session_player_unload(m_active_session->get_session());

    //Currently just round-robin.
    m_sess_it++;
    if (m_sess_it == m_session_cache.get<0>().end())
    {
        m_sess_it = m_session_cache.get<0>().begin();
    }

    m_active_session = m_sess_it->session;
    //should load track...
    //sp_session_player_load(m_active_session->get_session(), sometrack );

    update_timestamp();
    return;
}
