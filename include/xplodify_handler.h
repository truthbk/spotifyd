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

class XplodifyHandler
    : public SpotifyHandler
    , public Runnable
    , public Lockable
{
    public:
        XplodifyHandler()
            : SpotifyHandler()
            , Runnable()
            , Lockable(){
        }
        virtual ~XplodifyHandler() {
        }

        virtual void login(const std::string& username, const::string& passwd);
        virtual void login(const std::string& token);
        virtual bool login_status(std::string uuid);
        virtual std::string logout(std::string uuid);
        virtual std::vector< std::string > get_playlists(string uuid);
        virtual std::vector< std::string > get_tracks(string uuid, int pid);
        virtual bool select_playlist(std::string uuid, int pid);
        virtual bool select_playlist(std::string uuid, std::string pname);
        virtual bool select_track(std::string uuid, int tid);
        virtual bool select_track(std::string uuid, std::string tname);
        virtual void play();
        virtual void stop();

        virtual void notify_main_thread(void);
        virtual void set_playback_done(bool done);
        virtual int  music_playback(const sp_audioformat * format, 
                const void * frames, int num_frames);
        virtual void audio_fifo_stats(sp_audio_buffer_stats *stats);
        virtual void audio_fifo_flush_now(void) ;

        virtual int64_t get_session_state(std::string uuid);
        virtual void update_timestamp(void);
        virtual std::string get_cachedir();

    protected:
        //implementing runnable
        virtual void run();
};

#endif //_XPLODIFY_HANDLER
