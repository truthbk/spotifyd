#ifndef _SPOTIFY_CUST_H
#define _SPOTIFY_CUST_H

#include "lockable.h"
#include "runnable.h"

extern "C" {
	#include <libspotify/api.h>
	#include "audio.h"
}

//forward declarations.
sp_playlist_callbacks pl_callbacks;
sp_session_callbacks session_callbacks;

/* --- Data --- */
#define _SOMESIZE 8
const uint8_t g_appkey[_SOMESIZE + 1] = "AABBCCDD";
const size_t g_appkey_size = _SOMESIZE;

//nice per session wrapper class
class SpotifySession {
    public:
        SpotifySession();
        ~SpotifySession();
        sp_session * getSession(void){
            return m_sess;
        };
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



// This baby here, the SpotifyHandler, should be a singleton. The main reason
// for this is the fact that we can only have one audio queue, and we only need one
// handler for incoming thrift requests. If we've got several  ongoing spotify 
// sessions for different users, the idea is to randomly play music from each of 
// their 'selected' playlists. 
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
        static SpotifyHandler& getInstance()
        {
            static SpotifyHandler instance;
            return instance;
        }
        void loginSession(SpotifyCredential& _return, const SpotifyCredential& cred);
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

        typedef std::pair <SpotifyCredential, boost::shared_ptr<SpotifySession> > sess_map_pair; 
        typedef std::map <SpotifyCredential, boost::shared_ptr<SpotifySession> > session_map;

    protected:
        //implementing runnable
        void run();
    private:
        SpotifyHandler();
        void SpotifyInitHandler(const uint8_t *appkey = g_appkey, const size_t appkey_size = g_appkey_size);
        SpotifyHandler(SpotifyHandler const&); //Dont implement
        void operator=(SpotifyHandler const&); //Dont implement

        boost::shared_ptr<SpotifySession> getSession(const SpotifyCredential& cred);
        boost::shared_ptr<SpotifySession> getActiveSession(void);
        void setActiveSession(boost::shared_ptr<SpotifySession> session);

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
        session_map::iterator                   m_sess_it;
        boost::shared_ptr<SpotifySession>       m_active_session;
        int                                     m_playback_done;
};
#endif
