#ifndef _XPLODIFY_SESS_HH
#define _XPLODIFY_SESS_HH

#include <cstdint>
#include <ctime>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread/mutex.hpp>

#include "lockable.h"

#include "spotify_handler.h"
#include "Spotify.h"

extern "C" {
	#include <libspotify/api.h>
}

//forward declaration
class XplodifyHandler;
class XplodifyPlaylistContainer;
class XplodifyPlaylist;
class XplodifyTrack;

//nice per session wrapper class
class XplodifySession 
    : public boost::enable_shared_from_this<XplodifySession> {

    public:
        friend class XplodifyHandler;
        ~XplodifySession();

        static boost::shared_ptr< XplodifySession > create(XplodifyHandler * h = NULL );

        int init_session(const uint8_t * appkey, size_t appkey_size);
        int init_session(const uint8_t * appkey, size_t appkey_size, 
                sp_session_callbacks * sess_cb);

        int get_playback_done(void){
            return m_playback_done;
        };
        void set_playback_done(int done){
            m_playback_done = done;
        };
        boost::shared_ptr<XplodifyTrack> get_track(std::string user);
        boost::shared_ptr<XplodifyTrack> get_track(void);
#if 0
        boost::shared_ptr<XplodifyPlaylist> get_active_playlist(void){
            return m_playlist;
        };
        int getCurrentTrackIdx(){
            return m_track_idx;
        };
#endif
        bool get_logged_in(std::string username);
        bool get_logged_in(){
            return m_logged_in;
        }
        void set_logged_in(bool logged){
            m_logged_in = logged;
        }
#if 0
        std::string get_uuid() const {
            return m_uuid;
        }
       void set_uuid(std::string uuid) {
           m_uuid = uuid;
        }
#endif
       std::uintptr_t get_spsession_ptr() const {
           return reinterpret_cast<std::uintptr_t>(m_session);
        }


       bool available(void);
       void login( const std::string& username
                 , const std::string& passwd
                 , bool remember=false );

       bool logout(std::string user, bool unload=true,  bool doflush=false);
       void logged_out();

        sp_session * get_sp_session();
        void set_track(std::string user, int idx);
        void set_track(std::string user, std::string trackname);
        void update_plcontainer(std::string user, bool cascade=false);
        boost::shared_ptr<XplodifyPlaylistContainer> get_pl_container(std::string user);
        boost::shared_ptr<XplodifyPlaylistContainer> get_pl_container(void);
        void set_active_playlist(std::string user, int idx);
        void set_active_playlist(std::string user, std::string plname);
        std::string get_playlist_name(void);
        std::string get_playlist_name(std::string user);
        void update_state_ts(void);
        int64_t get_state_ts(void);
        int64_t get_state_ts(std::string user);

        void set_mode(SpotifyCmd::type mode);
        SpotifyCmd::type get_mode(void);

        static XplodifySession * get_session_from_udata(sp_session * sp);

#if 0
        void selectPlaylist(const SpotifyCredential& cred, const std::string& playlist);
#endif
    protected:
        XplodifySession();
        XplodifySession(XplodifyHandler * h);

        void logged_in(sp_session *sess, sp_error error);
        void play_token_lost();
        void start_playback();
        void stop_playback();
        void userinfo_updated();
        void end_of_track();
        void notify_main_thread(sp_session * sess);
        void get_audio_buffer_stats(sp_audio_buffer_stats * stats);
        int  music_delivery(sp_session *sess, const sp_audioformat *format,
                const void *frames, int num_frames);

    private:

        sp_session * get_session(void){
            return m_session;
        };
        struct SessionStatus {
            boost::shared_ptr<XplodifyPlaylistContainer>  m_plcontainer;
            boost::shared_ptr<XplodifyPlaylist>           m_playlist;
            boost::shared_ptr<XplodifyTrack>              m_track;
#define NO_TRACK NULL
#define NO_TRACK_IDX -1 //not a valid libspotify index that's why we use it.
            std::time_t                                   m_ts;
            int                                           m_track_idx;
            bool                                          m_logged_in;
            bool                                          m_logged_earlier;
        };

        void flush(std::string const user);
        bool is_user_active();
        bool user_exists(std::string const user);

        // C Callbacks...
        static void SP_CALLCONV cb_logged_in(sp_session *session, sp_error error);
        static void SP_CALLCONV cb_logged_out(sp_session *session);
        static void SP_CALLCONV cb_metadata_updated(sp_session *session);
        static void SP_CALLCONV cb_connection_error(sp_session *session, sp_error error);
        static void SP_CALLCONV cb_streaming_error(sp_session *session, sp_error error);
        static void SP_CALLCONV cb_msg_to_user(sp_session *session, const char *message);
        static void SP_CALLCONV cb_log_msg(sp_session *session, const char *data);

        /**
         * This callback is called from an internal libspotify thread to ask us to
         * reiterate the main loop.
         *
         * We notify the main thread using a condition variable and a protected variable.
         *
         * @sa sp_session_callbacks#notify_main_thread
         */
        static void SP_CALLCONV cb_notify_main_thread(sp_session *session);
        static int  SP_CALLCONV cb_music_delivery(sp_session *session, 
                const sp_audioformat *format, const void *frames, int num_frames);
        static void SP_CALLCONV cb_play_token_lost(sp_session *session);
        static void SP_CALLCONV cb_end_of_track(sp_session *session);
        static void SP_CALLCONV cb_userinfo_updated(sp_session *session);
        static void SP_CALLCONV cb_start_playback(sp_session *session);
        static void SP_CALLCONV cb_stop_playback(sp_session *session);
        static void SP_CALLCONV cb_get_audio_buffer_stats(sp_session *session,
                sp_audio_buffer_stats *stats);


        sp_session *                                  m_session;
        std::map< std::string, SessionStatus >        m_statuses;
        std::string                                   m_active_user;

        sp_session_callbacks    session_callbacks;
        sp_session_config       m_spconfig;

        //pointer to notify handler of stuff
        SpotifyHandler * const  m_handler;

        bool                    m_logged_in;
        bool                    m_logging_in;
        bool                    m_logging_out;
        int                     m_playback_done;
        int                     m_remove_tracks;

        SpotifyCmd::type        m_mode;
        SpotifyCmd::type        m_playback;
        std::time_t             m_ts;

        boost::mutex            m_mutex;


};

#endif
