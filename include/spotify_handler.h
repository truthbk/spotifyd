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

class SpotifyHandler 
    : public Runnable
    , private Lockable
    //should it inherit from shared_from_this?
{
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

        virtual void login(const std::string& username, const::string& passwd) = 0;
        virtual void login(const std::string& token) = 0;
        virtual bool login_status(std::string uuid) = 0;
        virtual std::string logout(std::string uuid) = 0;
        virtual std::vector< std::string > get_playlists(string uuid) = 0;
        virtual std::vector< std::string > get_tracks(string uuid, int pid) = 0;
        virtual bool select_playlist(std::string uuid, int pid) = 0;
        virtual bool select_playlist(std::string uuid, std::string pname) = 0;
        virtual bool select_track(std::string uuid, int tid) = 0;
        virtual bool select_track(std::string uuid, std::string tname) = 0;
        virtual void play() = 0;
        virtual void stop() = 0;

        virtual void notify_main_thread(void) = 0 ;
        virtual void set_playback_done(bool done) = 0 ;
        virtual int  music_playback(const sp_audioformat * format, 
                const void * frames, int num_frames) = 0 ;
        virtual void audio_fifo_stats(sp_audio_buffer_stats *stats) = 0 ;
#if 0
        virtual void audio_fifo_flush_now(void) = 0 ;
#endif
        virtual int64_t get_session_state(std::string uuid) = 0;
        virtual void update_timestamp(void) = 0 ;
        virtual std::string get_cachedir() = 0 ;

    protected:
        //implementing runnable
        void run();

        const size_t                            LOGIN_TO;
        int                                     m_playback_done;
        int                                     m_notify_events;
        std::string                             m_sp_cachedir;
        std::time_t                             m_ts;
        boost::asio::io_service                 m_io;

        //SILENCE NUM SAMPLES THRESHOLD
        enum { SILENCE_N_SAMPLES = 8192 };

    private:
        boost::shared_ptr<XplodifySession> get_active_session(void);
        void set_active_session(boost::shared_ptr<XplodifySession> session);

        void login_timeout(const boost::system::error_code&,
                std::string uuid);

        boost::shared_ptr<XplodifySession>      m_active_session;

};

#endif //_SPOTIFY_HANDLER_H
