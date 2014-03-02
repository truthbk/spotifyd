#ifndef _SPOTIFY_PL_HH
#define _SPOTIFY_PL_HH

#include <cstdint>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>

#include "xplodify_track.h"
#include "cacheable.h"

//forward declarations
class XplodifySession;

//NOTE: should this be lockable? I'm not sure Spotify C api enfores thread safety: libspotify is not thread safe :S
class XplodifyPlaylist
    : private Lockable
    , public Cacheable
    , public boost::enable_shared_from_this<XplodifyPlaylist>
{
    public:
        XplodifyPlaylist(XplodifySession& sess, int idx);
        ~XplodifyPlaylist();

        bool load(sp_playlist * pl);
        bool load(sp_playlist * pl, int32_t pos);
        bool unload(bool cascade=true);
        void flush();
        bool load_tracks();
        bool update_track_ptrs();
        bool set_sp_playlist(sp_playlist * pl);
        bool is_loaded();
        std::string get_name(bool cache=false);
        void   set_index(int idx);
        int    get_index(bool cache=true);
        size_t get_num_tracks(bool cache=false);
        void   add_track(boost::shared_ptr<XplodifyTrack> tr);
        void   add_track(boost::shared_ptr<XplodifyTrack> tr, int pos);
        boost::shared_ptr<XplodifyTrack> get_track_at(size_t idx);
        boost::shared_ptr<XplodifyTrack> get_track(int pos, bool remove=false);
        boost::shared_ptr<XplodifyTrack> get_track(std::string name, bool remove=false);
        boost::shared_ptr<XplodifyTrack> get_next_track();
        static XplodifyPlaylist * get_playlist_from_udata(
                sp_playlist * pl, void * userdata);

        //Cacheable purely virtual methods
        void cache(void);
        void uncache(void);
        bool is_cached(void);

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
        boost::shared_ptr<XplodifyTrack> remove_track_from_cache(int idx);
        boost::shared_ptr<XplodifyTrack> remove_track_from_cache(std::string& name);

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

        struct track_entry {
            std::string _trackname;

            boost::shared_ptr<XplodifyTrack> track;

            track_entry( const std::string &trackname, boost::shared_ptr<XplodifyTrack> tr ) 
                : _trackname(trackname)
                , track(tr)
            {
                return;
            }
        };

        //maybe tagging would make code more readable, but I'm not a fan.
        typedef boost::multi_index_container<
            track_entry,
            boost::multi_index::indexed_by<
                boost::multi_index::random_access<>,
                boost::multi_index::hashed_unique< 
                    BOOST_MULTI_INDEX_MEMBER(track_entry, std::string, _trackname) >
            > > track_cache;

        typedef track_cache::nth_index<0>::type track_cache_by_rand;
        typedef track_cache::nth_index<1>::type track_cache_by_name;
        typedef track_cache_by_rand::iterator track_r_iterator;

        static const sp_playlist_callbacks             cbs;

        track_cache                                    m_track_cache;
        track_cache_by_rand::iterator                  m_it_idx;
        track_cache_by_name::iterator                  m_it_name;

        XplodifySession&                               m_session;
        std::vector<
            boost::shared_ptr<XplodifyTrack>
            >                                          m_pending_tracks;
        sp_playlist *                                  m_playlist;
        bool                                           m_loading;


        std::string                                    m_name;
        int                                            m_index;
        size_t                                         m_num_tracks;

        //LOAD_WAIT
        enum { LOAD_WAIT_MS = 200 };

};


#endif
