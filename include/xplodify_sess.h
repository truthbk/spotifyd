#ifndef _XPLODIFY_SESS_HH
#define _XPLODIFY_SESS_HH

#include <cstdint>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

extern "C" {
	#include <libspotify/api.h>
}

//forward declaration
class XplodifyHandler;

//nice per session wrapper class
class XplodifySession : 
    public boost::enable_shared_from_this<XplodifySession> {

    public:
        ~XplodifySession();

        static boost::shared_ptr< XplodifySession > create(XplodifyHandler * h = NULL );

        sp_session * getSession(void){
            return m_sess;
        };
        int initSession(const uint8_t * appkey, size_t appkey_size);

        int getPlaybackDone(void){
            return m_playback_done;
        };
        void setPlaybackDone(int done){
            m_playback_done = done;
        };
        sp_playlist * getActivePlaylist(void){
            return m_jukeboxlist;
        };
        sp_track * getCurrentTrack(){
            return m_currenttrack;
        };
        int getCurrentTrackIdx(){
            return m_track_idx;
        };
        bool getLoggedIn(){
            return m_loggedin;
        }
        void setLoggedIn(bool logged){
            m_loggedin = logged;
        }
        std::string getUuid() const {
            return m_uuid;
        }
       void setUuid(std::string uuid) {
           m_uuid = uuid;
        }
       std::uintptr_t get_spsession_ptr() const {
           return reinterpret_cast<std::uintptr_t>(m_sess);
        }


       void login( const std::string& username
                 , const std::string& passwd
                 , bool remember=false );


        sp_session * get_sp_session();
        sp_track * setCurrentTrack(int idx);
        sp_playlistcontainer * getPlaylistContainer(void);
        void setActivePlaylist(sp_playlist * pl);
        std::string getPlaylistName(void);

        static XplodifySession * getSessionFromUData(sp_session * sp);

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
        int  music_delivery(sp_session *sess, const sp_audioformat *format,
                const void *frames, int num_frames);

    private:

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


        sp_session *            m_sess;
        sp_session_callbacks    session_callbacks;
        sp_playlist *           m_jukeboxlist;
        sp_track *              m_currenttrack;
        sp_session_config       m_spconfig;

        //pointer to notify handler of stuff
        XplodifyHandler * const  m_handler;

        std::string             m_uuid;
        bool                    m_loggedin;
        int                     m_notify_do;
        int                     m_playback_done;
        int                     m_remove_tracks;
#define NO_TRACK NULL
#define NO_TRACK_IDX -1 //not a valid libspotify index that's why we use it.
        int                     m_track_idx;
        

};

#endif