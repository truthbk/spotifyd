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
    , m_session()
    , m_playback_done(1)
    , m_notify_events(0)
    , m_sp_cachedir(SP_CACHEDIR)
    , m_ts(std::time(NULL))
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
    return;
}
bool XplodifyIPCServer::set_slave() {
    //TODO: method body
    return;
}
void XplodifyIPCServer::login(const SpotifyIPCCredential& cred) {
    //TODO: method body
    return;
}
bool XplodifyIPCServer::logout() {
    //TODO: method body
    return;
}
bool XplodifyIPCServer::is_logged(const SpotifyIPCCredential& cred){
    //TODO: method body
    return;
}
void XplodifyIPCServer::selectPlaylist(const std::string& playlist) {
    //TODO: method body
    return;
}
void XplodifyIPCServer::selectPlaylistById(const int32_t plist_id) {
    //TODO: method body
    return;
}
void XplodifyIPCServer::selectTrack(const std::string& track){
    //TODO: method body
    return;
}
void XplodifyIPCServer::selectTrackById(const int32_t track_id){
    //TODO: method body
    return;
}
void XplodifyIPCServer::play(){
    //TODO: method body
    return;
}
void XplodifyIPCServer::stop(){
    //TODO: method body
    return;
}
void XplodifyIPCServer::terminate_proc() {
    //TODO: method body
    return;
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
        m_session->end_of_track();
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
