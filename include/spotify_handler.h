#ifndef _SPOTIFY_HANDLER_H
#define _SPOTIFY_HANDLER_H

#include <cstdint>
#include <ctime>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "spotify_data.h"

extern "C" {
    #include <libspotify/api.h>
    #include "audio.h"
}

class SpotifyHandler {
    public:
        SpotifyHandler() 
            : LOGIN_TO(1)
            , m_playback_done(1)
            , m_notify_events(0)
            , m_sp_cachedir(SP_CACHEDIR)
            , m_ts(std::time(NULL)) {
        }
        virtual ~SpotifyHandler() {
        }

        virtual void notify_main_thread(void) = 0 ;
        virtual void set_playback_done(bool done) = 0 ;
        virtual int  music_playback(const sp_audioformat * format, 
                const void * frames, int num_frames) = 0 ;
        virtual void audio_fifo_stats(sp_audio_buffer_stats *stats) = 0 ;
#if 0
        virtual void audio_fifo_flush_now(void) = 0 ;
#endif
        virtual void update_timestamp(void) = 0 ;
        virtual std::string get_cachedir() = 0 ;

    protected:
        const size_t                            LOGIN_TO;
        int                                     m_playback_done;
        int                                     m_notify_events;
        std::string                             m_sp_cachedir;
        std::time_t                             m_ts;
        boost::asio::io_service                 m_io;

        //SILENCE NUM SAMPLES THRESHOLD
        enum { SILENCE_N_SAMPLES = 8192 };
};

#endif //_SPOTIFY_HANDLER_H
