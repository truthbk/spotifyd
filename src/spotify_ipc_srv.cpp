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
    , m_session(NULL)
    , m_playback_done(1)
    , m_notify_events(0)
    , m_sp_cachedir(SP_CACHEDIR)
    , m_ts(std::time(NULL))
    , m_multi(true)
    , m_master(false)
    , m_timer(m_io)
{

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
void XplodifyIPCServer::login(const SpotifyIPCCredential& cred) {
    //TODO: method body

    m_session = XplodifySession::create(NULL);
    if(m_session->init_session(g_appkey, g_appkey_size )) {
        //can't continue....
#ifdef _DEBUG
        std::cout << "Unexpected error creating session. "<< std::endl;
#endif
        m_session.reset();
        return;
    }

    m_session->login(cred._username, cred._passwd);

    m_timer.expires_from_now(boost::posix_time::seconds(LOGIN_TO));
    m_timer.async_wait(boost::bind(&XplodifyIPCServer::login_timeout,
                this, boost::asio::placeholders::error));


    return;
}
bool XplodifyIPCServer::logout() {
    //TODO: method body
    return true;
}
bool XplodifyIPCServer::is_logged(const SpotifyIPCCredential& cred){
    //TODO: method body
    return false;
}
void XplodifyIPCServer::selectPlaylist(const std::string& playlist) {

    if(!m_session) {
        return;
    }

    m_session->set_active_playlist(playlist);
    return;
}
void XplodifyIPCServer::selectPlaylistById(const int32_t plist_id) {

    if(!m_session) {
        return;
    }

    m_session->set_active_playlist(plist_id);
    return;
}
void XplodifyIPCServer::selectTrack(const std::string& track){

    if(!m_session) {
        return;
    }

    //m_session->set_track(track);
    return;
}
void XplodifyIPCServer::selectTrackById(const int32_t track_id){

    if(!m_session) {
        return;
    }

    //m_session->set_track(track_id);
    return;
}

void XplodifyIPCServer::play(){

    if(!m_session) {
        return;
    }
    //m_session->start_playback();

    return;
}
void XplodifyIPCServer::stop(){
    if(!m_session) {
        return;
    }
    //m_session->stop_playback();

    return;
}
void XplodifyIPCServer::terminate_proc() {
    lock();
    m_done = true;
    unlock();
    return;
}

void XplodifyIPCServer::notify_main_thread(void)
{
    lock();
    m_notify_events = 1;
    cond_signal();
    unlock();
}

void XplodifyIPCServer::set_playback_done(bool done)
{
    lock();
    m_playback_done = done;
    unlock();
}

audio_fifo_t * XplodifyIPCServer::audio_fifo() {
    return &m_audiofifo;
}

int XplodifyIPCServer::music_playback(const sp_audioformat *format,
	const void *frames, int num_frames)
{
    size_t s;
    audio_fifo_data_t *afd;

    if (num_frames == 0)
    {
        return 0; // Audio discontinuity, do nothing
    }

    //we're receiving synthetic "end of track" silence...
    if (num_frames > SILENCE_N_SAMPLES) {
        //m_session->end_of_track();
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

void XplodifyIPCServer::audio_fifo_stats(sp_audio_buffer_stats *stats)
{

    pthread_mutex_lock(&m_audiofifo.mutex);

    stats->samples = m_audiofifo.qlen;
    stats->stutter = 0; //how do we calculate this?

    pthread_cond_signal(&m_audiofifo.cond);
    pthread_mutex_unlock(&m_audiofifo.mutex);

}

void XplodifyIPCServer::audio_fifo_flush_now(void) {
    audio_fifo_flush(audio_fifo());
}


void XplodifyIPCServer::update_timestamp(void) {
    lock();
    m_ts = std::time(NULL);
    unlock();
}

std::string XplodifyIPCServer::get_cachedir() {
    return m_sp_cachedir;
}

void XplodifyIPCServer::run() {
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
            //protecting iterator
            lock();
            sp_session_process_events(m_session->get_session(), &next_timeout);
            unlock();
        } while (next_timeout == 0);

        m_io.poll();
        m_io.reset();

        lock();
    }
}


void XplodifyIPCServer::login_timeout(const boost::system::error_code& e) {
    //check session status...

    if(!m_session) {
        return;
    }

    if(m_session->get_logged_in()) {
#ifdef _DEBUG
        std::cout << " Succesfully logged in ...\n";
#endif
        return;
    }
    lock();
    //m_session->flush();
    m_session.reset();
    unlock();

    update_timestamp();
}



#if 0
/* --------------------------  SESSION CALLBACKS  ------------------------- */
void SP_CALLCONV XplodifySession::cb_logged_in(
        sp_session *sess, sp_error error) {

    XplodifySession * s = XplodifySession::get_session_from_udata(sess);
    if(!s) {
        return;
    }

    s->logged_in(sess, error);
    return;
}

void SP_CALLCONV XplodifySession::cb_logged_out(sp_session *sess) {

    //TODO
    return;
}

void SP_CALLCONV XplodifySession::cb_metadata_updated(sp_session *sess) {

    //TODO
    XplodifySession * s = XplodifySession::get_session_from_udata(sess);
    if(!s) {
        return;
    }
}

void SP_CALLCONV XplodifySession::cb_connection_error(
        sp_session *sess, sp_error error) {

    //TODO
    XplodifySession * s = XplodifySession::get_session_from_udata(sess);
    if(!s) {
        return;
    }
}

void SP_CALLCONV XplodifySession::cb_streaming_error(
        sp_session *sess, sp_error error) {

    //TODO
    XplodifySession * s = XplodifySession::get_session_from_udata(sess);
    if(!s) {
        return;
    }
}

void SP_CALLCONV XplodifySession::cb_msg_to_user(
        sp_session *sess, const char * message) {

    //TODO
    XplodifySession * s = XplodifySession::get_session_from_udata(sess);
    if(!s) {
        return;
    }
}

void SP_CALLCONV XplodifySession::cb_log_msg(
        sp_session *sess, const char * data) {

    //TODO
    XplodifySession * s = XplodifySession::get_session_from_udata(sess);
    if(!s) {
        return;
    }
}

void SP_CALLCONV XplodifySession::cb_notify_main_thread(sp_session *sess)
{

    XplodifySession * s = XplodifySession::get_session_from_udata(sess);
    if(!s) {
        return;
    }

    s->notify_main_thread(sess);
    return;
}

int SP_CALLCONV XplodifySession::cb_music_delivery(
        sp_session *sess, const sp_audioformat *format,
        const void *frames, int num_frames)
{
    int frames_consumed = 0;

    XplodifySession * s = XplodifySession::get_session_from_udata(sess);
    if(!s) {
        return 0;
    }

    frames_consumed = 
        s->music_delivery(sess, format, frames, num_frames);
    return frames_consumed; //or whatever...
}

void SP_CALLCONV XplodifySession::cb_play_token_lost(sp_session *sess) {

    XplodifySession * s = XplodifySession::get_session_from_udata(sess);
    if(!s) {
        return;
    }

    s->play_token_lost();
    return;
}

void SP_CALLCONV XplodifySession::cb_end_of_track(sp_session * sess) {
    //sometimes takes for ever to get called by libspotify. deprecating it...
#if 0
    XplodifySession * s = XplodifySession::get_session_from_udata(sess);
    if(!s) {
        return;
    }

    s->end_of_track();
#endif
    return;

}

void SP_CALLCONV XplodifySession::cb_userinfo_updated(sp_session * sp) {
    //TODO
    XplodifySession * s = XplodifySession::get_session_from_udata(sp);
    if(!s) {
        return;
    }
    s->userinfo_updated();
    return;
}
void SP_CALLCONV XplodifySession::cb_start_playback(sp_session * sp) {
    //TODO
    XplodifySession * s = XplodifySession::get_session_from_udata(sp);
    if(!s) {
        return;
    }
    s->start_playback();
    return;
}
void SP_CALLCONV XplodifySession::cb_stop_playback(sp_session * sp) {
    //TODO
    XplodifySession * s = XplodifySession::get_session_from_udata(sp);
    if(!s) {
        return;
    }
    s->stop_playback();
    return;
}
void SP_CALLCONV XplodifySession::cb_get_audio_buffer_stats(sp_session * sp,
        sp_audio_buffer_stats *stats) {

    XplodifySession * s = XplodifySession::get_session_from_udata(sp);
    if(!s) {
        return;
    }
    s->get_audio_buffer_stats(stats);
    return;
}
#endif
