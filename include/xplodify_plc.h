#ifndef _SPOTIFY_PLC_HH
#define _SPOTIFY_PLC_HH

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

#include "xplodify_pl.h"

//NOTE: should this be lockable? I'm not sure Spotify C api enfores thread safety.
class XplodifyPlaylistContainer : 
    public boost::enable_shared_from_this<XplodifyPlaylistContainer>
{
    public:
        XplodifyPlaylistContainer(XplodifySession& sess);
        ~XplodifyPlaylistContainer();

        bool    set_plcontainer(sp_playlistcontainer *plc);
        bool    load(sp_playlistcontainer * plc);
        bool    unload(bool cascade=true);
        void    flush();
        bool    add_playlist(XplodifyPlaylist * pl);
        bool    add_playlist(boost::shared_ptr<XplodifyPlaylist> pl);
        bool    add_playlist(boost::shared_ptr<XplodifyPlaylist> pl, int pos);
        bool    gen_starred();
        void    update_playlist_ptrs(bool cascade=false);
        void    update_pending_cache();
        size_t  get_num_playlists();
        boost::shared_ptr<XplodifyPlaylist> get_playlist(size_t idx);
        boost::shared_ptr<XplodifyPlaylist> get_playlist(std::string name);

        static XplodifyPlaylistContainer * get_plcontainer_from_udata(
                sp_playlistcontainer * plc, void * userdata);
    protected:
        void playlist_added(sp_playlist *pl, int pos);
        void playlist_removed(sp_playlist *pl, int pos);
        void playlist_moved(sp_playlist *pl, int pos, int newpos);
        void container_loaded();
    private:
        bool cache_has_key(std::string key);

        static void SP_CALLCONV cb_playlist_added(
                sp_playlistcontainer *pc, sp_playlist *pl, int pos, void *userdata);
        static void SP_CALLCONV cb_playlist_removed(
                sp_playlistcontainer *pc, sp_playlist *pl, int pos, void *userdata);
        static void SP_CALLCONV cb_playlist_moved(
                sp_playlistcontainer *pc, sp_playlist *pl, int pos, int newpos, void *userdata);
        static void SP_CALLCONV cb_container_loaded(
                sp_playlistcontainer * pc, void *userdata);


//Possibly a multi-index-container is overkill given the relatively small number 
//of playlists, but that's just how we roll..
        struct pl_entry {
            std::string _plname;

            boost::shared_ptr<XplodifyPlaylist> _playlist;

            pl_entry( const std::string &plname, boost::shared_ptr<XplodifyPlaylist> pl ) 
                : _plname(plname)
                , _playlist(pl)
            {
                return;
            }
        };

        //maybe tagging would make code more readable, but I'm not a fan. Leaving out for now.
        typedef boost::multi_index_container<
            pl_entry,
            boost::multi_index::indexed_by<
                boost::multi_index::random_access<>,
                boost::multi_index::hashed_unique< 
                    BOOST_MULTI_INDEX_MEMBER(pl_entry, std::string, _plname) >
            > > pl_cache;

        typedef pl_cache::nth_index<0>::type pl_cache_by_rand;
        typedef pl_cache::nth_index<1>::type pl_cache_by_name;

        static const sp_playlistcontainer_callbacks cbs;

        pl_cache                           m_pl_cache;
        std::vector<
            boost::shared_ptr<XplodifyPlaylist>
            >                              m_pending_playlists;
        //This is likely expendable.
        std::vector<
            boost::shared_ptr<XplodifyPlaylist>
            >                              m_failed_playlists;

        sp_playlistcontainer *             m_plcontainer;
        boost::shared_ptr<XplodifyPlaylist> 
                                           m_starred;
        XplodifySession&                   m_session;
        bool                               m_loading;
};

#endif
