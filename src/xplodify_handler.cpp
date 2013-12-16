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
    , m_sess_it(m_session_cache.get<0>().begin()) 
{
    //check temp dir.
    boost::filesystem::path dir(get_cachedir());
    if(!boost::filesystem::exists(dir)) {
        if(!boost::filesystem::create_directories(dir)) {
            //TODO: cleanup
            exit(1);
        }
    }
}

XplodifyHandler::~XplodifyHandler() {
    //EMPTY.
}

bool XplodifyHandler::handler_available(){
    if(m_active_session) {
        return false;
    }

    return true;
}

std::string XplodifyHandler::check_in(){
    boost::shared_ptr< XplodifySession > sess = XplodifySession::create(this);
    if(sess->init_session(g_appkey, g_appkey_size )) {
#ifdef _DEBUG
        std::cout << "Unexpected error creating session. "<< std::endl;
#endif
        sess.reset();
        return std::string(); //empty string if error.
    }


    lock();
    //generate UUID
    const boost::uuids::uuid uuid = boost::uuids::random_generator()();
    const std::string uuid_str = boost::lexical_cast<std::string>(uuid);

    m_session_cache.get<1>().insert(sess_map_entry( uuid_str, 
                const_cast<const sp_session *>(sess->get_session()), sess ));

    sess->set_uuid(uuid_str);
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

    remove_from_cache(uuid);
    return true;
}

//Returning true means login process has been initiated, not succesful.
bool XplodifyHandler::login(const std::string& uuid,
        const std::string& username, const std::string& passwd){

    boost::shared_ptr< XplodifySession > sess = get_session(uuid);
    if(!sess) {
        //user not checked in.
        return false;
    }

    if(sess->get_logged_in()) {
        return true;
    }

    sess->login(username, passwd);
    lock();
    set_active_session(sess);
    unlock();

    return true;

}

bool XplodifyHandler::login(const std::string& uuid, const std::string& token){

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
    bool switched = false;

    boost::shared_ptr<XplodifySession> sess = get_session(uuid);
    if(!sess) {
        return false;
    }

    lock();
    if(sess != m_active_session) 
    {
        //already logged out..
        return true;

    }

    //switch_session();
    sess->logout();

#ifdef _DEBUG
    std::cout << "XplodifySession use count: " << sess.use_count()  << std::endl;
#endif
    unlock();
    return true;
}

std::vector< boost::shared_ptr<XplodifyPlaylist> > XplodifyHandler::get_playlists(
        std::string uuid){

    std::vector< boost::shared_ptr<XplodifyPlaylist> > pls;

    boost::shared_ptr<XplodifySession> sess = get_session(uuid);
    if(!sess) {
        return pls;
    }

    //TODO: !!Important!! what if cached?
    boost::shared_ptr<XplodifyPlaylistContainer> pc = sess->get_pl_container();
    if(!pc) {
        return pls;
    }

    int n =  pc->get_num_playlists();
    for (int i = 0; i<n; ++i) {
        boost::shared_ptr<XplodifyPlaylist> pl = pc->get_playlist(i);
        pls.push_back(pl);
    }

    return pls;
}
std::vector< boost::shared_ptr<XplodifyTrack> > XplodifyHandler::get_tracks(
        std::string uuid, int pid){

    std::vector<boost::shared_ptr<XplodifyTrack> > playlist;

    boost::shared_ptr<XplodifySession> sess = get_session(uuid);
    if(!sess) {
        return playlist;
    }

    boost::shared_ptr<XplodifyPlaylistContainer> pc = sess->get_pl_container();
    if(!pc) {
        return playlist;
    }

    boost::shared_ptr<XplodifyPlaylist> pl = pc->get_playlist(pid);

    if(!pl) {
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

    return playlist;
}
std::vector< boost::shared_ptr<XplodifyTrack> > XplodifyHandler::get_tracks(
        std::string uuid, const std::string& name){

    std::vector<boost::shared_ptr<XplodifyTrack> > playlist;

    boost::shared_ptr<XplodifySession> sess = get_session(uuid);
    if(!sess) {
        return playlist;
    }

    boost::shared_ptr<XplodifyPlaylistContainer> pc = sess->get_pl_container();
    if(!pc) {
        return playlist;
    }
    boost::shared_ptr<XplodifyPlaylist> pl = pc->get_playlist(name);

    if(!pl) {
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

    return playlist;

}

bool XplodifyHandler::select_playlist(std::string uuid, int pid){
    boost::shared_ptr< XplodifySession > sess = get_session(uuid);
    if(!sess) {
        return false;
    }

    sess->set_active_playlist(pid);
    return true;
}

bool XplodifyHandler::select_playlist(std::string uuid, std::string pname){
    boost::shared_ptr<XplodifySession> sess = get_session(uuid);
    if(!sess) {
        return false;
    }

    sess->set_active_playlist(pname);
    return true;
}
bool XplodifyHandler::select_track(std::string uuid, int tid){
    boost::shared_ptr< XplodifySession > sess = get_session(uuid);
    if(!sess) {
        return false;
    }
#if 0
#ifdef _DEBUG
    std::cout << "Selecting track " << track << " for session with uuid: " 
        << sess->m_uuid << std::endl;
#endif
#endif
    sess->set_track(tid);
    return true;
}
bool XplodifyHandler::select_track(std::string uuid, std::string tname){
    boost::shared_ptr< XplodifySession > sess = get_session(uuid);
    if(!sess) {
        return false;
    }
#if 0
#ifdef _DEBUG
    std::cout << "Selecting track " << track << " for session with uuid: " 
        << sess->m_uuid << std::endl;
#endif
#endif
    sess->set_track(tname);
    return true;

}
boost::shared_ptr<XplodifyTrack> XplodifyHandler::whats_playing(std::string uuid) {
    //TODO: check if uuid is indeed checked-in
    if(!exists_in_cache(uuid) || !m_active_session) {
        return boost::shared_ptr<XplodifyTrack>();
    }

    boost::shared_ptr<XplodifyTrack> tr( m_active_session->get_track());
    return tr;

}

void XplodifyHandler::play(){
    m_active_session->start_playback();
}

void XplodifyHandler::stop(){
    m_active_session->stop_playback();
    audio_fifo_flush_now();
    audio_fifo_set_reset(audio_fifo(), 1);
}

void XplodifyHandler::notify_main_thread(void){
    lock();
    m_notify_events = 1;
    cond_signal();
    unlock();
}

void XplodifyHandler::set_session_done(bool done){
    lock();
    m_session_done = done;
    unlock();
}

void XplodifyHandler::set_playback_done(bool done){
    lock();
    m_playback_done = done;
    unlock();
}

int  XplodifyHandler::music_playback(const sp_audioformat * format, 
        const void * frames, int num_frames) {
    size_t s;
    audio_fifo_data_t *afd;

    if (num_frames == 0)
    {
        return 0; // Audio discontinuity, do nothing
    }

    //we're receiving synthetic "end of track" silence...
    if (num_frames > SILENCE_N_SAMPLES) {
        m_active_session->end_of_track();
        pthread_mutex_unlock(&m_audiofifo.mutex);
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

int64_t XplodifyHandler::get_session_state(std::string uuid){
    boost::shared_ptr<XplodifySession> sess = get_session(uuid);
    if(!sess) {
        return 0;
    }

    return sess->get_state_ts();
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
            if(!m_active_session) {
                sleep(0);
            } if(m_session_done) {
                m_active_session.reset();
                m_session_done = 0;
            }else {
                sp_session_process_events(m_active_session->get_session(), &next_timeout);
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

boost::shared_ptr<XplodifySession> XplodifyHandler::get_session(const std::string& uuid) {

    sess_map_by_uuid& sess_by_uuid = m_session_cache.get<1>();

    sess_map_by_uuid::iterator sit = sess_by_uuid.find(uuid);
    if( sit == m_session_cache.get<1>().end() ) {
        return boost::shared_ptr<XplodifySession>();
    }

    return sit->session;
}

boost::shared_ptr<XplodifySession> XplodifyHandler::get_session(const sp_session * sps) {

    sess_map_by_sessptr& sessByPtr = m_session_cache.get<2>();

    sess_map_by_sessptr::iterator sit = sessByPtr.find(reinterpret_cast<uintptr_t>(sps));
    if( sit == m_session_cache.get<2>().end() ) {
        return boost::shared_ptr<XplodifySession>();
    }

    return sit->session;
}

void XplodifyHandler::remove_from_cache(const std::string& uuid) {
    bool move_on = false;
    sess_map_entry aux_entry(*m_sess_it);
    sess_map_sequenced::iterator sess_it = m_sess_it;
    if(++sess_it == m_session_cache.get<0>().end()) {
        sess_it = m_session_cache.get<0>().begin();
    }
    if(aux_entry._uuid == uuid) {
        move_on = true;
    }
    sess_map_by_uuid& sess_by_uuid = m_session_cache.get<1>();
    size_t n = sess_by_uuid.erase(uuid);

    //fix the potentially invalidated iterator.
    if(n) {
        if(move_on) {
            m_sess_it = sess_it;
        } else {
            m_sess_it = m_session_cache.get<0>().iterator_to(aux_entry);
        }
    }
}

bool XplodifyHandler::exists_in_cache(const std::string& uuid) {

    sess_map_by_uuid& sess_by_uuid = m_session_cache.get<1>();
    sess_map_by_uuid::iterator sit = sess_by_uuid.find(uuid);
    if( sit == m_session_cache.get<1>().end() ) {
        return false;
    }

    return true;
}
