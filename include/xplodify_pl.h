#ifndef _SPOTIFY_PL_HH
#define _SPOTIFY_PL_HH

#include <cstdint>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

extern "C" {
	#include <libspotify/api.h>
}

class XplodifyPlaylist : 
    public boost::enable_shared_from_this<XplodifyPlaylist>
{
    public:
        static XplodifyPlaylist * getPlaylistFromUData(void * userdata);

    protected:
        void tracks_added(
                sp_track *const *tracks, int num_tracks, 
                int position, void *userdata);
        void tracks_removed(
                const int *tracks, int num_tracks, void *userdata);
        void tracks_moved(
                const int *tracks, int num_tracks, 
                int new_position, void *userdata);
        void playlist_renamed(void *userdata);
        void playlist_state_changed(void *userdata);
        void playlist_update_in_progress(bool done, void *userdata);
        void playlist_metadata_updated(void *userdata);
        void track_created_changed(int plosition, sp_user *user, int when, void *userdata);
        void track_seen_changed(int position, bool seen, void *userdata);
        void description_changed(const char *desc, void *userdata);
        void image_changed(const byte *image, void *userdatadata);
        void track_message_changed(int position, const char *message, void *userdata);
        void subscribers_changed(void *userdata);

    private:
        static void SP_CALLCONV cb_tracks_added(
                sp_playlist *pl, sp_track *const *tracks, int num_tracks, 
                int position, void *userdata);
        static void SP_CALLCONV cb_tracks_removed(
                sp_playlist *pl, const int *tracks, int num_tracks, void *userdata);
        static void SP_CALLCONV cb_tracks_moved(
                sp_playlist *pl, const int *tracks, int num_tracks, 
                int new_position, void *userdata);
        static void SP_CALLCONV cb_playlist_renamed(
                sp_playlist *pl, void *userdata);
        static void SP_CALLCONV cb_playlist_state_changed(
                sp_playlist *pl, void *userdata);
        static void SP_CALLCONV cb_playlist_update_in_progress(
                sp_playlist *pl, bool done, void *userdata);
        static void SP_CALLCONV cb_playlist_metadata_updated(
                sp_playlist *pl, void *userdata);
        static void SP_CALLCONV cb_track_created_changed(
                sp_playlist *pl, int plosition, sp_user *user, int when, void *userdata);
        static void SP_CALLCONV cb_track_seen_changed(
                sp_playlist *pl, int position, bool seen, void *userdata);
        static void SP_CALLCONV cb_description_changed(
                sp_playlist *pl, const char *desc, void *userdata);
        static void SP_CALLCONV cb_image_changed(
                sp_playlist *pl, const byte *image, void *userdatadata);
        static void SP_CALLCONV cb_track_message_changed(
                sp_playlist *pl, int position, const char *message, void *userdata);
        static void SP_CALLCONV cb_subscribers_changed(
                sp_playlist *pl, void *userdata);


        boost::shared_ptr<XplodifySession> m_session;

};
#endif
