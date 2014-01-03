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

#include "xplodify_handler.h"
#include "xplodify_sess.h"
#include "xplodify_plc.h"
#include "xplodify_pl.h"

#include "Spotify.h"

XplodifyHandler::XplodifyHandler()
    : SpotifyHandler() 
{
    //check temp dir.
    boost::filesystem::path dir(get_cachedir());
    if(!boost::filesystem::exists(dir)) {
        if(!boost::filesystem::create_directories(dir)) {
            //TODO: cleanup
            exit(1);
        }
    }
    enum audio_arch arch;
#ifdef _OSX
#ifdef HAS_OPENAL
    arch = OPENAL_ARCH;
#elif HAS_AUDIOTOOLKIT
    arch = AUDIOTOOLKIT;
#endif
#else
#ifdef _LINUX
    arch = ALSA;
#endif
#endif
    set_audio(arch);
    audio_init(&m_audiofifo);

    //create session

    if(m_session.init_session(g_appkey, g_appkey_size )) {
#ifdef _DEBUG
        std::cout << "Unexpected error creating session. "<< std::endl;
#endif
        exit(1); //NASTY!
    }
}

XplodifyHandler::~XplodifyHandler() {
    //EMPTY.
}

bool XplodifyHandler::handler_available(){
    //TODO: accomodate to new architecture.
    return true;
}

std::string XplodifyHandler::check_in(){

    //generate UUID
    const boost::uuids::uuid uuid = boost::uuids::random_generator()();
    const std::string uuid_str = boost::lexical_cast<std::string>(uuid);

    return uuid_str;
}

bool XplodifyHandler::check_out(const std::string& uuid){

    if(is_checked_in(uuid)) {
        //user not checked in.
        return false;
    }

    lock();
    std::string username(get_username(uuid));
    m_session.logout(username);
    m_session.flush(username);
    unlock();

    remove_from_cache(uuid);
    return true;
}

//Returning true means login process has been initiated, not succesful.
bool XplodifyHandler::login(const std::string& uuid,
        const std::string& username, const std::string& passwd){

    if(is_checked_in(uuid)) {
        //user not checked in.
        return false;
    }

    if(m_session.get_logged_in(username)) {
        return true;
    }

    lock();
    m_user_cache.get<1>().insert(
            user_entry( uuid, username, passwd));


    if(m_session.available()) {
        m_session.login(username, passwd, true);
    } else {
        unlock();
        return false;
    }

    unlock();
    return true;

}

bool XplodifyHandler::login(const std::string& uuid, const std::string& token){

    if(is_checked_in(uuid)) {
        //user not checked in.
        return false;
    }
    // MORE TODO

    return true;
}

bool XplodifyHandler::login_status(std::string uuid){

    if(is_checked_in(uuid)) {
        //user not checked in.
        return false;
    }

    std::string username(get_username(uuid));

    return m_session.get_logged_in(username);
}

//only the active session can be logged in
bool XplodifyHandler::logout(std::string uuid){
    bool switched = false;

    if(is_checked_in(uuid)) {
        //user not checked in.
        return false;
    }

    lock();
    std::string username(get_username(uuid));

    m_session.logout(username, false);
    unlock();

    return true;
}

std::vector< boost::shared_ptr<XplodifyPlaylist> > XplodifyHandler::get_playlists(
        std::string uuid){

    std::vector< boost::shared_ptr<XplodifyPlaylist> > pls;

    if(is_checked_in(uuid)) {
        //user not checked in.
        return pls;
    }

    std::string username(get_username(uuid));

    //TODO: !!Important!! what if cached?
    lock();
    boost::shared_ptr<XplodifyPlaylistContainer> pc = 
        m_session.get_pl_container(username);
    if(!pc) {
        return pls;
    }

    int n =  pc->get_num_playlists();
    for (int i = 0; i<n; ++i) {
        boost::shared_ptr<XplodifyPlaylist> pl = pc->get_playlist(i);
        pls.push_back(pl);
    }
    unlock();

    return pls;
}
std::vector< boost::shared_ptr<XplodifyTrack> > XplodifyHandler::get_tracks(
        std::string uuid, int pid){

    std::vector<boost::shared_ptr<XplodifyTrack> > playlist;

    if(is_checked_in(uuid)) {
        //user not checked in.
        return playlist;
    }

    std::string username(get_username(uuid));

    lock();
    boost::shared_ptr<XplodifyPlaylistContainer> pc = 
        m_session.get_pl_container(username);
    if(!pc) {
        unlock();
        return playlist;
    }

    boost::shared_ptr<XplodifyPlaylist> pl = pc->get_playlist(pid);

    if(!pl) {
        unlock();
        return playlist;
    }

    for(unsigned int j = 0 ; j < pl->get_num_tracks() ; j++ ) {
        boost::shared_ptr<XplodifyTrack> tr = pl->get_track_at(j);
#ifdef _DEBUG
        if(!tr->is_loaded()) {
            std::cout << "Track at index: "<<  j << " is loading" << std::endl;
        }
#endif
        playlist.push_back(tr);
    }

    unlock();
    return playlist;
}
std::vector< boost::shared_ptr<XplodifyTrack> > XplodifyHandler::get_tracks(
        std::string uuid, const std::string& name){

    std::vector<boost::shared_ptr<XplodifyTrack> > playlist;

    if(is_checked_in(uuid)) {
        //user not checked in.
        return playlist;
    }

    std::string username(get_username(uuid));

    lock();
    boost::shared_ptr<XplodifyPlaylistContainer> pc =
        m_session.get_pl_container(username);
    if(!pc) {
        unlock();
        return playlist;
    }
    boost::shared_ptr<XplodifyPlaylist> pl = pc->get_playlist(name);

    if(!pl) {
        unlock();
        return playlist;
    }

    for(unsigned int j = 0 ; j < pl->get_num_tracks() ; j++ ) {
        boost::shared_ptr<XplodifyTrack> tr = pl->get_track_at(j);
#ifdef _DEBUG
        if(!tr->is_loaded()) {
            std::cout << "Track at index: "<<  j << " is loading" << std::endl;
        }
#endif
        playlist.push_back(tr);
    }

    unlock();
    return playlist;

}

bool XplodifyHandler::select_playlist(std::string uuid, int pid){

    if(is_checked_in(uuid)) {
        //user not checked in.
        return false;
    }

    std::string username(get_username(uuid));
    m_session.set_active_playlist(username, pid);

    return true;
}

bool XplodifyHandler::select_playlist(std::string uuid, std::string pname){

    if(is_checked_in(uuid)) {
        //user not checked in.
        return false;
    }

    std::string username(get_username(uuid));
    m_session.set_active_playlist(username, pname);

    return true;
}

bool XplodifyHandler::select_track(std::string uuid, int tid){

    if(is_checked_in(uuid)) {
        //user not checked in.
        return false;
    }
#if 0
#ifdef _DEBUG
    std::cout << "Selecting track " << track << " for session with uuid: " 
        << sess->m_uuid << std::endl;
#endif
#endif

    std::string username(get_username(uuid));
    m_session.set_track(username, tid);

    return true;
}
bool XplodifyHandler::select_track(std::string uuid, std::string tname){

    if(is_checked_in(uuid)) {
        //user not checked in.
        return false;
    }
#if 0
#ifdef _DEBUG
    std::cout << "Selecting track " << track << " for session with uuid: " 
        << sess->m_uuid << std::endl;
#endif
#endif

    std::string username(get_username(uuid));
    m_session.set_track(username, tname);

    return true;

}
boost::shared_ptr<XplodifyTrack> XplodifyHandler::whats_playing(std::string uuid) {
    //TODO: check if uuid is indeed checked-in
    if(!is_checked_in(uuid) || m_session.available()) {
        return boost::shared_ptr<XplodifyTrack>();
    }

    boost::shared_ptr<XplodifyTrack> tr( m_session.get_track());

    return tr;

}

void XplodifyHandler::play(){

    m_session.start_playback();
    update_timestamp();
}

void XplodifyHandler::stop(){

    m_session.stop_playback();

    audio_fifo_flush_now();
    audio_fifo_set_reset(audio_fifo(), 1);
    update_timestamp();
}

void XplodifyHandler::next(){
    //this is different for multi session
    m_session.end_of_track();

    update_timestamp();
}

void XplodifyHandler::prev(){
    //this is different for multi session
    m_session.end_of_track();

    update_timestamp();
}

void XplodifyHandler::notify_main_thread(void){
    lock();
    m_notify_events = 1;
    cond_signal();
    unlock();
}

void XplodifyHandler::set_playback_done(bool done){
    lock();
    m_playback_done = done;
    unlock();
    update_timestamp();
}

int XplodifyHandler::music_playback(const sp_audioformat * format, 
        const void * frames, int num_frames) {
    size_t s;
    audio_fifo_data_t *afd;

    if (num_frames == 0)
    {
        return 0; // Audio discontinuity, do nothing
    }

    //we're receiving synthetic "end of track" silence...
    if (num_frames > SILENCE_N_SAMPLES) {
        lock();
        m_session.end_of_track();
        unlock();

        update_timestamp();
        return 0;
    }

    pthread_mutex_lock(&m_audiofifo.mutex);

    /* Buffer one second of audio, no more */
    if (m_audiofifo.qlen > format->sample_rate)
    {
#ifdef _DEBUG
        std::cout << "[INFO] Frames in audio_queue: " << m_audiofifo.qlen << std::endl;
#endif
        pthread_mutex_unlock(&m_audiofifo.mutex);
        return 0;
    }

    //buffer underrun
    if( m_audiofifo.prev_qlen && !m_audiofifo.qlen) 
    {
        m_audiofifo.reset = 1;
#ifdef _DEBUG
        std::cout << "[WARNING] Buffer underrun detected." << std::endl;
#endif
    }

    s = num_frames * sizeof(int16_t) * format->channels;

    //dont want to malloc, change this to new....
    afd = (audio_fifo_data_t *) malloc(sizeof(audio_fifo_data_t) + s);
    memcpy(afd->samples, frames, s);
    afd->nsamples = num_frames;
    afd->rate = format->sample_rate;
    afd->channels = format->channels;

    TAILQ_INSERT_TAIL(&m_audiofifo.q, afd, link);
    m_audiofifo.prev_qlen = m_audiofifo.qlen;
    m_audiofifo.qlen += num_frames;

#ifdef _DEBUG
    std::cout << "[INFO] Frames fed: " << num_frames << std::endl;
    std::cout << "[INFO] Frames in audio_queue: " << m_audiofifo.qlen << std::endl;
#endif
    pthread_cond_signal(&m_audiofifo.cond);
    pthread_mutex_unlock(&m_audiofifo.mutex);
    return num_frames;
}

void XplodifyHandler::audio_fifo_stats(sp_audio_buffer_stats *stats){
    pthread_mutex_lock(&m_audiofifo.mutex);

    stats->samples = m_audiofifo.qlen;
    stats->stutter = 0; //how do we calculate this?

    pthread_cond_signal(&m_audiofifo.cond);
    pthread_mutex_unlock(&m_audiofifo.mutex);
    return;
}

void XplodifyHandler::audio_fifo_flush_now(void){
    audio_fifo_flush(audio_fifo());
    return;
}

int64_t XplodifyHandler::get_handler_state(){
    int64_t ts = 0;
    lock();
    ts = m_ts;
    unlock();

    return ts;
}

int64_t XplodifyHandler::get_session_state(std::string uuid){

    if(is_checked_in(uuid)) {
        //user not checked in.
        return 0;
    }
    std::string username(get_username(uuid));

    return m_session.get_state_ts(username);
}

void XplodifyHandler::update_timestamp(void){
    lock();
    m_ts = std::time(NULL);
    unlock();
}

std::string XplodifyHandler::get_cachedir(){
    return m_sp_cachedir;
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
            lock();
            if(m_session.available()) {
                sleep(0);
            } else {
                sp_session_process_events(m_session.get_sp_session(), &next_timeout);
            }
            unlock();
        } while (next_timeout == 0);

#if 0
        m_io.poll();
        m_io.reset();
#endif

        lock();
    }
}


std::string XplodifyHandler::get_username(const std::string& uuid) {

    user_map_by_uuid& user_by_uuid = m_user_cache.get<1>();

    user_map_by_uuid::iterator sit = user_by_uuid.find(uuid);
    if( sit == m_user_cache.get<1>().end() ) {
        return std::string();
    }

    return sit->_user;
}


void XplodifyHandler::remove_from_cache(const std::string& uuid) {
#if 0
    bool move_on = false;
    user_entry aux_entry(*m_user_it);
    user_map_sequenced::iterator user_it = m_user_it;
    if(++user_it == m_user_cache.get<0>().end()) {
        user_it = m_user_cache.get<0>().begin();
    }
    if(aux_entry._uuid == uuid) {
        move_on = true;
    }
    user_map_by_uuid& sess_by_uuid = m_user_cache.get<1>();
    size_t n = user_by_uuid.erase(uuid);

    //fix the potentially invalidated iterator.
    if(n) {
        if(move_on) {
            m_user_it = user_it;
        } else {
            m_user_it = m_user_cache.get<0>().iterator_to(aux_entry);
        }
    }
#endif
}

bool XplodifyHandler::is_checked_in(const std::string& uuid) {

    user_map_by_uuid& sess_by_uuid = m_user_cache.get<1>();
    user_map_by_uuid::iterator sit = sess_by_uuid.find(uuid);
    if( sit == m_user_cache.get<1>().end() ) {
        return false;
    }

    return true;
}
