#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

#include "Spotify.h"
#include "xplodify_plc.h"
#include "xplodify_sess.h"

extern "C" {
    #include <libspotify/api.h>
}

const sp_playlistcontainer_callbacks XplodifyPlaylistContainer::cbs = {
    cb_playlist_added,
    cb_playlist_removed,
    cb_playlist_moved,
    cb_container_loaded
};

XplodifyPlaylistContainer::XplodifyPlaylistContainer(
        boost::shared_ptr<XplodifySession> sess)
    : m_plcontainer(NULL)
    , m_session(sess)
    , m_loading(false) 
{
    //EMPTY
}

XplodifyPlaylistContainer::~XplodifyPlaylistContainer(){
    //TODO: make sure we cleanup properly.
#ifdef _DEBUG
    std::cout << "Xplodify Playlist Container destroyed...." << std::endl;
#endif
}

bool XplodifyPlaylistContainer::load(sp_playlistcontainer * plc) {
    if(!plc) {
        return false;
    }

    m_plcontainer = plc;
    sp_playlistcontainer_add_callbacks(plc, 
            const_cast<sp_playlistcontainer_callbacks *>(&cbs), this);

    m_loading = !sp_playlistcontainer_is_loaded(plc);
    if(!m_loading) {
#ifdef _DEBUG
        std::cout << "container was immediately loaded" << std::endl;
#endif
        container_loaded();
    }
    return m_loading;
}
bool XplodifyPlaylistContainer::unload() {

    if(!m_plcontainer){
        return true;
    }

    sp_playlistcontainer_remove_callbacks(
            m_plcontainer, const_cast<sp_playlistcontainer_callbacks *>(&cbs), this);
    m_pl_cache.get<0>().clear();
    m_plcontainer = NULL;

    return true;
}

void XplodifyPlaylistContainer::add_playlist(XplodifyPlaylist * pl) {
    if(!pl) {
        return;
    }
    boost::shared_ptr<XplodifyPlaylist> xpl(pl);
    add_playlist(xpl);
}

void XplodifyPlaylistContainer::add_playlist(boost::shared_ptr<XplodifyPlaylist> pl) {
    if(!pl) {
        return;
    }

    //do this with exceptions once this is rolling.
    std::string name(pl->get_name());
    if(!name.empty()) {
        m_pl_cache.get<0>().push_back(pl_entry(name, pl));
    }
}

void XplodifyPlaylistContainer::add_playlist(boost::shared_ptr<XplodifyPlaylist> pl, int pos) {
    if(!pl) {
        return;
    }

    //get iterator...
    pl_cache_by_rand& c_r = m_pl_cache.get<0>();
    pl_cache_by_rand::iterator it = c_r.iterator_to(c_r[pos]);

    //do this with exceptions once this is rolling.
    std::string name(pl->get_name());
    if(!name.empty()) {
            c_r.insert(it, pl_entry(name, pl));
    }
}

size_t XplodifyPlaylistContainer::get_num_playlists() {
    pl_cache_by_rand& c_r = m_pl_cache.get<0>();

    return c_r.size();
}

boost::shared_ptr<XplodifyPlaylist> 
XplodifyPlaylistContainer::get_playlist(size_t idx) {

    if(idx >  get_num_playlists()-1) {
        return boost::shared_ptr<XplodifyPlaylist>();
    }
    pl_cache_by_rand& c_r = m_pl_cache.get<0>();

    return c_r[idx]._playlist;
}

boost::shared_ptr<XplodifyPlaylist> 
XplodifyPlaylistContainer::get_playlist(std::string name) {

    pl_cache_by_name& c_n = m_pl_cache.get<1>();

    pl_cache_by_name::iterator it = c_n.find(name);

    if(it == m_pl_cache.get<1>().end() ) {
        return boost::shared_ptr<XplodifyPlaylist>();
    }

    return boost::shared_ptr<XplodifyPlaylist>(it->_playlist);

}

void XplodifyPlaylistContainer::flush() {

    pl_cache_by_rand& c_r = m_pl_cache.get<0>();

    pl_cache_by_rand::iterator it = c_r.begin();

    while(it != m_pl_cache.get<0>().end() ) {
        boost::shared_ptr<XplodifyPlaylist> pl = it->_playlist;
#ifdef _DEBUG
        std::cout << "Flushing " << pl->get_name() << std::endl;
#endif
        it = c_r.erase(it);

        //flush the tracks from the playlist as well...
        pl->flush();
        pl.reset();
    }
}

void XplodifyPlaylistContainer::playlist_added(sp_playlist *pl, int pos){
    //log this.
    if(m_loading) {
#ifdef _DEBUG
        std::cout << "Playlist Container still loading... easy!" << std::endl;
#endif
        return;
    }

    boost::shared_ptr<XplodifySession> sess(m_session.lock());
    boost::shared_ptr<XplodifyPlaylist> npl(new XplodifyPlaylist(sess));
    npl->load(pl);
    if(npl->is_loaded()) {
#ifdef _DEBUG
        std::cout << "Playlist " << npl->get_name() << " loaded into plc." << std::endl;
#endif
        npl->cache();
        add_playlist(npl, pos);
    }

    return;
}

void XplodifyPlaylistContainer::playlist_removed(sp_playlist *pl, int pos){

    pl_cache_by_name& c = m_pl_cache.get<1>();
    c.erase(std::string(sp_playlist_name(pl)));
}

//might have to lock().
void XplodifyPlaylistContainer::playlist_moved(sp_playlist *pl, int pos, int newpos){

    //put in the right place in the rand index...
    pl_cache_by_name& c_n = m_pl_cache.get<1>();

    pl_cache_by_name::iterator it = c_n.find(sp_playlist_name(pl));

    //shouldn't happen
    if(it == m_pl_cache.get<1>().end()) {
        return;
    }

    boost::shared_ptr<XplodifyPlaylist> xpl(it->_playlist);
    //remove it from the list
    c_n.erase(it);

    //add it in the new position.
    add_playlist(xpl, newpos-1);
    return;
}

void XplodifyPlaylistContainer::container_loaded(){

    //container_loaded callback never being raised....
    m_loading = false;

    int n = sp_playlistcontainer_num_playlists(m_plcontainer);
#ifdef _DEBUG
    std::cout << "Playlist container loaded succesfully. Contains " 
        << n << " playlists" << std::endl;
#endif
    for(int i=0 ; i<n ; i++ ) {
        sp_playlist * p = sp_playlistcontainer_playlist( m_plcontainer, i);

        boost::shared_ptr<XplodifySession> sess(m_session.lock());
        boost::shared_ptr<XplodifyPlaylist> npl(new XplodifyPlaylist(sess));
        npl->load(p);
        if(npl->is_loaded()) {
#ifdef _DEBUG
            std::cout << "Playlist " << npl->get_name() << " loaded into plc." << std::endl;
#endif
            npl->cache();
            add_playlist(npl);
        }
    }
    return;
}


XplodifyPlaylistContainer * XplodifyPlaylistContainer::get_plcontainer_from_udata(
        sp_playlistcontainer * plc, void * userdata) {
    XplodifyPlaylistContainer * plcptr = 
        reinterpret_cast<XplodifyPlaylistContainer *>(userdata);

    if(plcptr->m_plcontainer != plc) {
        return NULL;
    }

    return plcptr;
}


void SP_CALLCONV XplodifyPlaylistContainer::cb_playlist_added(
        sp_playlistcontainer *pc, sp_playlist *pl, int pos, void *userdata) {

    XplodifyPlaylistContainer * xplc = 
        XplodifyPlaylistContainer::get_plcontainer_from_udata(pc, userdata);
    if(!xplc) {
        return;
    }

    xplc->playlist_added(pl, pos);
}

void SP_CALLCONV XplodifyPlaylistContainer::cb_playlist_removed(
        sp_playlistcontainer *pc, sp_playlist *pl, int pos, void *userdata) {

    XplodifyPlaylistContainer * xplc = 
        XplodifyPlaylistContainer::get_plcontainer_from_udata(pc, userdata);
    if(!xplc) {
        return;
    }

    xplc->playlist_removed(pl, pos);
}

void SP_CALLCONV XplodifyPlaylistContainer::cb_playlist_moved(
        sp_playlistcontainer *pc, sp_playlist *pl, int pos, int newpos, void *userdata){

    XplodifyPlaylistContainer * xplc = 
        XplodifyPlaylistContainer::get_plcontainer_from_udata(pc, userdata);
    if(!xplc) {
        return;
    }

    xplc->playlist_moved(pl, pos, newpos);
}

void SP_CALLCONV XplodifyPlaylistContainer::cb_container_loaded(
        sp_playlistcontainer * pc, void *userdata){

#ifdef _DEBUG
    std::cout << "Container Loaded CB called." << std::endl;
#endif

    XplodifyPlaylistContainer * xplc = 
        XplodifyPlaylistContainer::get_plcontainer_from_udata(pc, userdata);
    if(!xplc) {
        return;
    }

    xplc->container_loaded();
}
