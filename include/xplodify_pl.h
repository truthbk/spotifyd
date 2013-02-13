#ifndef _SPOTIFY_PL_HH
#define _SPOTIFY_PL_HH

#include <cstdint>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>

extern "C" {
	#include <libspotify/api.h>
}

//forward declarations
class XplodifySession;
class XplodifyPlaylist;


class XplodifyPlaylistContainer : 
    public boost::enable_shared_from_this<XplodifyPlaylistContainer>
{
    public:
        XplodifyPlaylistContainer();
        ~XplodifyPlaylistContainer();

        static XplodifyPlaylistContainer * getPlaylistContainerFromUData(void * userdata);
    protected:
        void playlist_added(sp_playlist *pl, int pos, void *userdata);
        void playlist_removed(sp_playlist *pl, int pos, void *userdata);
        void playlist_moved(sp_playlist *pl, int pos, int newpos, void *userdata);
        void container_loaded(void *userdata);
    private:
        static void SP_CALLCONV cb_playlist_added(
                sp_playlistcontainer *pc, sp_playlist *pl, int pos, void *userdata);
        static void SP_CALLCONV cb_playlist_removed(
                sp_playlistcontainer *pc, sp_playlist *pl, int pos, void *userdata);
        static void SP_CALLCONV cb_playlist_moved(
                sp_playlistcontainer *pc, sp_playlist *pl, int pos, int newpos, void *userdata);
        static void SP_CALLCONV cb_container_loaded(
                sp_playlistcontainer * pc, void *userdata);

        struct pl_entry {
            std::string _plname;
            int _pos;

            boost::shared_ptr<XplodifyPlaylist> playlist;

            pl_entry( const std::string &plname, int pos
                    , boost::shared_ptr<XplodifyPlaylist> pl ) 
                : _plname(plname)
                , _pos(pos)
                , playlist(pl)
            {
                return;
            }
        };

        //maybe tagging would make code more readable, but I'm not a fan. Leaving out for now.
        typedef boost::multi_index_container<
            pl_entry,
            boost::multi_index::indexed_by<
                boost::multi_index::sequenced<>,
                boost::multi_index::hashed_unique< 
                    BOOST_MULTI_INDEX_MEMBER(pl_entry, std::string, _plname) >
            > > pl_map;

        typedef pl_map::nth_index<0>::type pl_map_sequenced;
        typedef pl_map::nth_index<1>::type pl_map_by_name;

        pl_map                m_pl_cache;
        sp_playlistcontainer * m_playlist;
        bool                  loading;
};

class XplodifyPlaylist : 
    public boost::enable_shared_from_this<XplodifyPlaylist>
{
    public:
        XplodifyPlaylist(boost::shared_ptr<XplodifySession> sess);
        ~XplodifyPlaylist();

        static XplodifyPlaylist * getPlaylistFromUData(void * userdata);

    protected:
        void tracks_added(
                sp_track *const *tracks, int num_tracks, 
                int position);
        void tracks_removed(const int *tracks, int num_tracks);
        void tracks_moved(const int *tracks, int num_tracks, int new_position);
        void playlist_renamed();
        void playlist_state_changed();
        void playlist_update_in_progress(bool done);
        void playlist_metadata_updated();
        void track_created_changed(int position, sp_user *user, int when);
        void track_seen_changed(int position, bool seen);
        void description_changed(const char *desc);
        void image_changed(const byte *image);
        void track_message_changed(int position, const char *message);
        void subscribers_changed();

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
        sp_playlist *                      m_playlist;
        bool                               loading;

};
#endif
