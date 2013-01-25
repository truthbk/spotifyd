#ifndef _SPOTIFY_CUST_H
#define _SPOTIFY_CUST_H

#include <set>

#include "lockable.h"
#include "runnable.h"

extern "C" {
	#include <libspotify/api.h>
	#include "audio.h"
}

/* --- Data --- */
const uint8_t g_appkey[] = {
0x01, 0xD7, 0x2B, 0x8C, 0xC7, 0xB9, 0xBE, 0xBF, 0x88, 0x45, 0xBE, 0x56, 0xDE, 0xCA, 0x4A, 0x11,
	0xB9, 0x6B, 0xF9, 0x3F, 0xA2, 0x77, 0x9A, 0xD3, 0x68, 0xE9, 0x4F, 0x1C, 0xFF, 0x6D, 0x70, 0x41,
	0x2A, 0x35, 0x6E, 0x1D, 0x87, 0x37, 0xA3, 0x21, 0xFD, 0xFA, 0x37, 0x8D, 0xDA, 0xE4, 0xD9, 0x98,
	0x4D, 0x07, 0x01, 0x31, 0x6B, 0xEB, 0x35, 0xAC, 0xAA, 0x4A, 0xF2, 0x04, 0xB2, 0x8C, 0x81, 0x31,
	0xF9, 0x9E, 0x9C, 0x84, 0x1B, 0xB7, 0x48, 0x93, 0x34, 0x8A, 0x45, 0x7F, 0xAA, 0xBD, 0x04, 0x60,
	0x72, 0x78, 0x06, 0x3E, 0x97, 0xDA, 0x36, 0x5D, 0xBB, 0x0A, 0x37, 0x69, 0x27, 0x3B, 0x49, 0x9D,
	0xB1, 0x68, 0x14, 0x80, 0x13, 0x50, 0xD5, 0x8D, 0xAD, 0xF9, 0xDF, 0xC0, 0xDE, 0x82, 0x7D, 0x42,
	0x69, 0x9F, 0xE5, 0xCB, 0x7A, 0x05, 0xE3, 0x88, 0x27, 0x18, 0x2C, 0x05, 0x1B, 0x04, 0x51, 0xA4,
	0x9F, 0x66, 0xDF, 0x4B, 0x39, 0xF7, 0x56, 0x8E, 0xC4, 0xB1, 0x81, 0xDE, 0xF0, 0x11, 0x9C, 0x2C,
	0x22, 0x69, 0xC8, 0x0D, 0x6F, 0xAB, 0x8E, 0x6B, 0x60, 0x21, 0xF9, 0x64, 0xD2, 0x85, 0x7F, 0x65,
	0x6F, 0x97, 0x53, 0x62, 0x78, 0x7B, 0x79, 0x8A, 0x83, 0x3C, 0x66, 0xA2, 0x97, 0xC4, 0x0A, 0xAB,
	0x30, 0xA4, 0x37, 0x2D, 0xE0, 0xDF, 0xD8, 0x68, 0x91, 0x72, 0xCA, 0x36, 0xD5, 0xEB, 0xA1, 0x5A,
	0x76, 0xE8, 0xAF, 0x24, 0xFC, 0x64, 0x74, 0x69, 0x76, 0xE6, 0x30, 0xC8, 0xFE, 0xA1, 0x28, 0x25,
	0x90, 0x1F, 0x68, 0xBD, 0x2D, 0xF6, 0x75, 0x0A, 0x1D, 0xF5, 0x97, 0x24, 0x60, 0x39, 0x3C, 0xC0,
	0xC9, 0x52, 0x96, 0x0F, 0x9F, 0x0B, 0x20, 0x09, 0xD9, 0xDE, 0xE6, 0x8B, 0x69, 0x98, 0x99, 0x81,
	0xAD, 0x17, 0x23, 0x67, 0x30, 0x23, 0xD5, 0x9E, 0x50, 0x67, 0xB3, 0xB2, 0x9E, 0x66, 0xB2, 0x5F,
	0x21, 0x2B, 0xA9, 0x2B, 0xA2, 0x52, 0x9D, 0xCB, 0xF8, 0x1D, 0x94, 0x56, 0xAC, 0xA9, 0x63, 0xC8,
	0x5B, 0xF1, 0x74, 0xE1, 0xDB, 0x13, 0xF1, 0xB4, 0xC5, 0x79, 0x77, 0x94, 0x07, 0xF5, 0x1D, 0x08,
	0xC1, 0xE9, 0xBF, 0xF5, 0x72, 0x97, 0x4D, 0xE2, 0xFB, 0x3B, 0xE1, 0x1A, 0x5C, 0xF6, 0x73, 0xDA,
	0x53, 0x28, 0xA0, 0x29, 0xAF, 0xCC, 0x8E, 0x2D, 0xC0, 0x11, 0xD6, 0x4F, 0x7B, 0x9D, 0x14, 0x23,
	0x5B,};

const size_t g_appkey_size = sizeof(g_appkey);


//nice per session wrapper class
class SpotifySession {
    public:
        SpotifySession();
        ~SpotifySession();
        sp_session * getSession(void){
            return m_sess;
        };
        int initSession(const sp_session_config *cfg);
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

        sp_track * setCurrentTrack(int idx);
        sp_playlistcontainer * getPlaylistContainer(void);
        void setActivePlaylist(sp_playlist * pl);
        std::string getPlaylistName(void);
#if 0
        void selectPlaylist(const SpotifyCredential& cred, const std::string& playlist);
#endif
    protected:
        void run();
    private:
        sp_session *m_sess;
        int m_notify_do;
        int m_playback_done;
        sp_playlist *m_jukeboxlist;
        int m_remove_tracks;
#define NO_TRACK NULL
#define NO_TRACK_IDX -1 //not a valid libspotify index that's why we use it.
        sp_track *m_currenttrack;
        int m_track_idx;

};

#if 0
template <typename T>
struct lthelper
{
    bool operator()(const T * t1, const T * t2)
    {
        return ((t1-t2) < 0);
    }
}
#endif



// This baby here, the SpotifyHandler, was originally conceived as a singleton. The 
// main idea behind that decision was the fact that we are only supposed to have a 
// single audio queue. Although perhaps fundamentally correct, because singleton's are
// very often a bad decision which may bring nast consequences eventually, I rather leave 
// that decision to the coder - if he decides to to implement several  SpotifyHandler, 
// instances.... then he will be responsible for his actions ;)
//
// The Handler manages incoming thrift requests and most of the logic.
// If we've got several ongoing spotify sessions for different users, the idea is to 
// randomly play music from each of their 'selected' playlists. 
//
// For example: if Joe is listening to his Hip Hop playist, and Jane is listening
// to her oldies playlists. We *do not* want several instances of the SpotifyHandler
// created. We don't need them either. The Handler will randomly select tracks from
// each of the users registered. The Audio Queue must be a singleton, and so we 
// can make the entire handler singleton. One instance is enough to handler all
// incoming requests.
//
class SpotifyHandler 
        : virtual public SpotifyIf
        , public Runnable
        , private Lockable {
    public:
        SpotifyHandler();
        void loginSession(SpotifyCredential& _return, const SpotifyCredential& cred);
        bool isLoggedIn(const SpotifyCredential& cred);
        void logoutSession(const SpotifyCredential& cred);
        void sendCommand(const SpotifyCredential& cred, const SpotifyCmd::type cmd);
        void switchSession();
        void tracks_added(sp_playlist *pl, sp_track * const *tracks,
                int num_tracks, int position, void *userdata);
        void tracks_removed(sp_playlist *pl, const int *tracks,
                int num_tracks, void *userdata);
        void tracks_moved(sp_playlist *pl, const int *tracks,
                int num_tracks, int new_position, void *userdata);
        void playlist_renamed(sp_playlist *pl, void *userdata);
        void logged_in(sp_session *sess, sp_error error);
        void end_of_track(sp_session *sess);
        void play_token_lost(sp_session *sess);
        int music_delivery(sp_session *sess, const sp_audioformat *format,
                const void *frames, int num_frames);
        void notify_main_thread(sp_session * sess);
        void search(SpotifyPlaylist& _return, const SpotifyCredential& cred,
			const SpotifySearch& criteria);
        void getPlaylists(SpotifyPlaylistList& _return, const SpotifyCredential& cred);
        void getPlaylist(SpotifyPlaylist& _return, const SpotifyCredential& cred,
			const int32_t plist_id);
        void getPlaylistByName(SpotifyPlaylist& _return, const SpotifyCredential& cred,
			const std::string& name);
        void selectPlaylist(const SpotifyCredential& cred, const std::string& playlist);
        void selectPlaylistById(const SpotifyCredential& cred, const int32_t plist_id);
        bool merge2playlist(const SpotifyCredential& cred, const std::string& pl,
			const SpotifyPlaylist& tracks);
        bool add2playlist(const SpotifyCredential& cred, const std::string& pl,
			const SpotifyTrack& track);
        void whats_playing(SpotifyTrack& _return);

        //consider multindex container?
        typedef std::pair <SpotifyCredential, boost::shared_ptr<SpotifySession> > sess_map_pair; 
        typedef std::map <
            SpotifyCredential, boost::shared_ptr<SpotifySession> >::iterator sess_map_it; 
        typedef std::map <SpotifyCredential, boost::shared_ptr<SpotifySession> > session_map;

        typedef std::pair <sp_session *, boost::shared_ptr<SpotifySession> > csess_map_pair; 
        typedef std::map <sp_session *, boost::shared_ptr<SpotifySession> > csession_map;

    protected:
        //implementing runnable
        void run();
    private:
        void SpotifyInitHandler(const uint8_t *appkey = g_appkey,
                const size_t appkey_size = g_appkey_size);

        boost::shared_ptr<SpotifySession> getSession(const SpotifyCredential& cred);
        boost::shared_ptr<SpotifySession> getActiveSession(void);
        void setActiveSession(boost::shared_ptr<SpotifySession> session);

        //Callbacks...
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

        //we also need to be able to search by sp_session, that's quite important; callbacks rely very heavily
        //on it.
        sp_playlistcontainer *                  getPlaylistContainer(SpotifyCredential& cred);
        session_map&                            sessions();
        audio_fifo_t *                          audio_fifo();
        sp_session_config *                     app_config();

        //libspotify wrapped
        sp_session_config                       m_spconfig;
        audio_fifo_t                            m_audiofifo;

        //proper members
        session_map                             m_sessions;
        csession_map                            m_csessions;
        session_map::iterator                   m_sess_it;
        csession_map::iterator                  m_csess_it;
        boost::shared_ptr<SpotifySession>       m_active_session;
        std::set<sp_session *>                  m_event_spsessions;

        int                                     m_playback_done;
        int                                     m_notify_events;

        sp_playlist_callbacks                   pl_callbacks;
        sp_session_callbacks                    session_callbacks;
};
#endif
