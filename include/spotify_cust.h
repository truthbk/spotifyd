#ifndef _SPOTIFY_CUST_H
#define _SPOTIFY_CUST_H

#include "lockable.h"

extern "C" {
	#include <libspotify/api.h>
	#include "audio.h"
}

/* --- Data --- */
extern const uint8_t g_appkey[];
extern const size_t g_appkey_size;

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
private:
  sp_session *m_sess;
  int m_notify_do;
  int m_playback_done;
  sp_playlist *m_jukeboxlist;
  int m_remove_tracks;
  sp_track *m_currenttrack;
  int m_track_idx;

};


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
class SpotifyHandler :
	virtual public SpotifyIf, private Lockable {
    public:
        static SpotifyHandler * getInstance();
        void loginSession(SpotifyCredential& _return, const SpotifyCredential& cred);
        void logoutSession(const SpotifyCredential& cred);
        void sendCommand(const SpotifyCredential& cred, const SpotifyCmd::type cmd);
        void switchSession();
        void tracks_added_cb(sp_playlist *pl, sp_track * const *tracks,
                int num_tracks, int position, void *userdata);
        void tracks_removed_cb(sp_playlist *pl, const int *tracks,
                int num_tracks, void *userdata);
        void tracks_moved_cb(sp_playlist *pl, const int *tracks,
                int num_tracks, int new_position, void *userdata);
        void playlist_renamed_cb(sp_playlist *pl, void *userdata);
        void logged_in_cb(sp_session *sess, sp_error error);
        void end_of_track_cb(sp_session *sess);
        void play_token_lost_cb(sp_session *sess);
        int music_delivery_cb(sp_session *sess, const sp_audioformat *format,
                const void *frames, int num_frames);
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
    private:
        SpotifyHandler(const uint8_t *appkey = g_appkey, const size_t appkey_size = g_appkey_size);
        typedef std::map< boost::shared_ptr<SpotifyCredential>, boost::shared_ptr<SpotifySession> > session_map;

        boost::shared_ptr<SpotifySession> getSession(SpotifyCredential& cred);
        boost::shared_ptr<SpotifySession> getActiveSession(void);
        void setActiveSession(boost::shared_ptr<SpotifySession> session);

        //we also need to be able to search by sp_session, that's quite important; callbacks rely very heavily
        //on it.
        sp_playlistcontainer * getPlaylistContainer(SpotifyCredential& cred);
        session_map& sessions();
        audio_fifo_t * audio_fifo();
        sp_session_config * app_config();

        //libspotify wrapped
        sp_session_config m_spconfig;
        audio_fifo_t m_audiofifo;

        //proper members
        session_map m_sessions;
        boost::shared_ptr<SpotifySession> m_active_session;
        session_map::const_iterator m_sess_it;
        static SpotifyHandler * m_handler_ptr;
};
#endif
