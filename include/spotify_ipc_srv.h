#ifndef _SPOTIFY_CUST_HH
#define _SPOTIFY_CUST_HH

//Boost.
#include <boost/shared_ptr.hpp>

#include "SpotifyIPC.h"


class XplodifyIPCServer
    : virtual public SpotifyIPCIf
    , public Runnable
    , private Lockable 
    , private SpotifyHandler {
{
    public:
        XplodifyIPCServer();

        bool set_master();
        bool set_slave();
        void login(const SpotifyIPCCredential& cred);
        bool logout();
        bool is_logged(const SpotifyIPCCredential& cred);
        void selectPlaylist(const std::string& playlist);
        void selectPlaylistById(const int32_t plist_id);
        void selectTrack(const std::string& track);
        void selectTrackById(const int32_t track_id);
        void play();
        void stop();
        void terminate_proc();

        void notify_main_thread(void) = 0 ;
        void set_playback_done(bool done);
        int  music_playback(const sp_audioformat * format, 
                const void * frames, int num_frames);
        void audio_fifo_stats(sp_audio_buffer_stats *stats);
        void audio_fifo_flush_now(void);
        void update_timestamp(void);
        std::string get_cachedir();

    protected:
        //implementing runnable
        void run();

    private:
        boost::shared_ptr<XplodifySession>      m_session;

        int                                     m_playback_done;
        int                                     m_notify_events;
        std::string                             m_sp_cachedir;
        std::time_t                             m_ts;
        const bool                              m_multi;
        bool                                    m_master;

        audio_fifo_t *                          audio_fifo();

        //libspotify wrapped
        audio_fifo_t                            m_audiofifo;


}

#endif //_SPOTIFY_CUST_HH
