#ifndef _SPOTIFY_HANDLER_H
#define _SPOTIFY_HANDLER_H

#include <cstdint>
#include <ctime>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "runnable.h"
#include "lockable.h"
#include "spotify_data.h"


extern "C" {
    #include <libspotify/api.h>
    #include "audio.h"
};

//Forward declaration.
class XplodifySession;
class XplodifyPlaylist;
class XplodifyTrack;

class SpotifyHandler 
    : public Runnable
    , public Lockable
{
    public:
        SpotifyHandler() 
            : Runnable()
            , Lockable()
            , LOGIN_TO(SP_TIMEOUT)
            , m_playback_done(1)
            , m_notify_events(0)
            , m_sp_cachedir(SP_CACHEDIR)
            , m_ts(std::time(NULL))
            , m_active_session() {
        }
        virtual ~SpotifyHandler() {
        }

        virtual bool handler_available() = 0;
        virtual std::string check_in() = 0;
        virtual bool check_out(const std::string& uuid) = 0;
        virtual bool login(const std::string& uuid, 
                const std::string& username, const std::string& passwd) = 0;
        virtual bool login(const std::string& uuid, const std::string& token) = 0;
        virtual bool login_status(std::string uuid) = 0;
        virtual bool logout(std::string uuid) = 0;
        virtual std::vector< 
            boost::shared_ptr<XplodifyPlaylist> > get_playlists(std::string uuid) = 0;
        virtual std::vector< 
            boost::shared_ptr<XplodifyTrack> > get_tracks(std::string uuid, int pid) = 0;
        virtual std::vector< 
            boost::shared_ptr<XplodifyTrack> > get_tracks(
                    std::string uuid, const std::string& name) = 0;
        virtual bool select_playlist(std::string uuid, int pid) = 0;
        virtual bool select_playlist(std::string uuid, std::string pname) = 0;
        virtual bool select_track(std::string uuid, int tid) = 0;
        virtual bool select_track(std::string uuid, std::string tname) = 0;
        virtual boost::shared_ptr<XplodifyTrack> whats_playing(std::string uuid) = 0;
        virtual void play() = 0;
        virtual void stop() = 0;

        virtual void notify_main_thread(void) = 0;
        virtual void set_playback_done(bool done) = 0;
        virtual int  music_playback(const sp_audioformat * format, 
                const void * frames, int num_frames) = 0;
        virtual void audio_fifo_stats(sp_audio_buffer_stats *stats) = 0;
        virtual void audio_fifo_flush_now(void) = 0 ;

        virtual int64_t get_session_state(std::string uuid) = 0;
        virtual void update_timestamp(void) = 0;
        virtual std::string get_cachedir() = 0;

    protected:
        //implementing runnable
        virtual void run() = 0;

        const size_t                            LOGIN_TO;
        int                                     m_playback_done;
        int                                     m_notify_events;
        std::string                             m_sp_cachedir;
        std::time_t                             m_ts;
        boost::asio::io_service                 m_io;

        //SILENCE NUM SAMPLES THRESHOLD
        enum { SILENCE_N_SAMPLES = 8192 };

        //libspotify wrapped
        audio_fifo_t                            m_audiofifo;
        audio_fifo_t *                          audio_fifo(){
            return &m_audiofifo;
        }


        virtual boost::shared_ptr<XplodifySession> get_active_session(void){
            return m_active_session;
        };
        virtual void set_active_session(boost::shared_ptr<XplodifySession> session){
            m_active_session = session;
        };
        void login_timeout(const boost::system::error_code&,
                std::string uuid);

        boost::shared_ptr<XplodifySession>      m_active_session;

};

#endif //_SPOTIFY_HANDLER_H
