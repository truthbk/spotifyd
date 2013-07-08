#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

//might have to sleep
#include <chrono>
#include <thread>

//boost
#include <boost/shared_ptr.hpp>

#include "Spotify.h"
#include "xplodify_pl.h"
#include "xplodify_sess.h"

extern "C" {
#include <libspotify/api.h>
}

const sp_playlist_callbacks XplodifyPlaylist::cbs = {
    cb_tracks_added,
    cb_tracks_removed,
    cb_tracks_moved,
    cb_playlist_renamed,
    cb_playlist_state_changed,
    cb_playlist_update_in_progress,
    cb_playlist_metadata_updated,
    cb_track_created_changed,
    cb_track_seen_changed,
    cb_description_changed,
    cb_image_changed,
    cb_track_message_changed,
    cb_subscribers_changed
};

XplodifyPlaylist::XplodifyPlaylist(boost::shared_ptr<XplodifySession> sess) 
    : m_session(sess)
    , m_playlist(NULL)
    , m_loading(true) {
    //EMPTY
}
XplodifyPlaylist::~XplodifyPlaylist() {
    //EMPTY
}

boost::shared_ptr<XplodifyTrack> 
XplodifyPlaylist::remove_track_from_cache(int idx) {

    track_cache_by_rand& tr_cache_rand = m_track_cache.get<0>();
    track_cache_by_rand::const_iterator cit = tr_cache_rand.begin();

    cit = cit+idx-1;
    boost::shared_ptr<XplodifyTrack> ret_track(cit->track);

    tr_cache_rand.erase(cit);
    return ret_track;
}

boost::shared_ptr<XplodifyTrack> 
XplodifyPlaylist::remove_track_from_cache(std::string& name){
    track_cache_by_name& tr_cache_name = m_track_cache.get<1>();
    track_cache_by_name::iterator it = tr_cache_name.find(name);


    if(it == tr_cache_name.end()) {
        return boost::shared_ptr<XplodifyTrack>();
    }
    boost::shared_ptr<XplodifyTrack> ret_track(it->track);

    tr_cache_name.erase(name);
    return ret_track;
}

bool XplodifyPlaylist::load(sp_playlist * pl) {
    if(!pl) {
        return false;
    }

    m_playlist = pl;
    sp_playlist_add_callbacks(pl, const_cast<sp_playlist_callbacks *>(&cbs), this);

    if(!sp_playlist_is_loaded(pl))
    {
        m_loading = true;
    } else {
        m_loading = false; //already loaded.
        load_tracks();
    }

    return true;
}

void XplodifyPlaylist::flush() {

    track_cache_by_rand& t_r = m_track_cache.get<0>();
    std::size_t sz = t_r.size();

    for(std::size_t i=0 ; i<sz ; i++) {
        boost::shared_ptr<XplodifyTrack> tr = get_track(0, true);
        tr.reset();
    }
}

bool XplodifyPlaylist::is_loaded() {
    return !m_loading;
}

bool XplodifyPlaylist::load_tracks() {
    int n;

    if(m_loading) {
        return false;
    }

    n = sp_playlist_num_tracks(m_playlist);
    for(int i=0 ; i<n ; i++) {
        sp_track * t = sp_playlist_track(m_playlist, i);

        track_cache_by_rand& tr_cache_rand = m_track_cache.get<0>();

        boost::shared_ptr<XplodifyTrack> tr(new XplodifyTrack(m_session));
        if(tr->load(t)){
#if 0
            std::chrono::milliseconds hiatus(LOAD_WAIT_MS);
            while(!tr->is_loaded()) {
                //sleep for a bit... HATE THIS.
                std::this_thread::sleep_for(hiatus);
            }
#endif
            tr_cache_rand.push_back(track_entry(tr->get_name(), tr));
#ifdef _DEBUG
            std::cout << "Track " << tr->get_name() << " loaded for playlist "
                << get_name() << std::endl;
#endif
        }
    }
    return true;
}

bool XplodifyPlaylist::unload() {

    if(!m_plcontainer){
        return true;
    }

    sp_playlist_remove_callbacks(m_playlist, const_cast<sp_playlist_callbacks *>(&cbs), this);
    m_track_cache.get<0>().clear();
    m_loading = false;
    m_playlist = NULL;

    return true;
}

//when we put in Exceptions this will be a lot cleaner.
std::string XplodifyPlaylist::get_name() {
    if(!m_playlist) {
        return std::string("");
    }

    return std::string(sp_playlist_name(m_playlist));
}

size_t XplodifyPlaylist::get_num_tracks(){
    track_cache_by_rand& t_r = m_track_cache.get<0>();

    return t_r.size();
}

boost::shared_ptr<XplodifyTrack> XplodifyPlaylist::get_track_at(size_t idx) {

    if(idx >=  get_num_tracks()) {
        return boost::shared_ptr<XplodifyTrack>();
    }
    track_cache_by_rand& c_r = m_track_cache.get<0>();

    return c_r[idx].track;
}

void XplodifyPlaylist::add_track(boost::shared_ptr<XplodifyTrack> tr) {
    if(!tr) {
        return;
    }

    track_cache_by_rand& t_r = m_track_cache.get<0>();
    //push_back() delivers better performance than insert.
    t_r.push_back(track_entry(tr->get_name(), tr));
}

void XplodifyPlaylist::add_track(boost::shared_ptr<XplodifyTrack> tr, int pos) {
    if(!tr) {
        return;
    }

    //get iterator...
    track_cache_by_rand& t_r = m_track_cache.get<0>();
    track_cache_by_rand::iterator it = t_r.iterator_to(t_r[pos]);

    t_r.insert(it, track_entry(tr->get_name(), tr));
}

boost::shared_ptr<XplodifyTrack> XplodifyPlaylist::get_track(int pos, bool remove) {
    boost::shared_ptr<XplodifyTrack> t;

    track_cache_by_rand& t_r = m_track_cache.get<0>();
    track_cache_by_rand::iterator it = t_r.iterator_to(t_r[pos]);

    if(it == t_r.end()) {
        t = boost::shared_ptr<XplodifyTrack>();
    } else {
        t = it->track;
    }
    if(remove) {
        t_r.erase(it);
    }

    return t;
}

boost::shared_ptr<XplodifyTrack> XplodifyPlaylist::get_track(std::string name, bool remove) {
    boost::shared_ptr<XplodifyTrack> t;

#ifdef _DEBUG
    std::cout << "Seeking for track " << name << " from playlist: " << get_name() << std::endl;
#endif
    track_cache_by_name& t_n = m_track_cache.get<1>();
    track_cache_by_name::iterator it = t_n.find(name);

    if(it == t_n.end()) {
        t = boost::shared_ptr<XplodifyTrack>();
    } else {
        t = it->track;
    }
    if(remove) {
        t_n.erase(it);
    }

    return t;
}


XplodifyPlaylist * XplodifyPlaylist::get_playlist_from_udata(
        sp_playlist * pl, void * userdata) {
    XplodifyPlaylist * plptr = 
        reinterpret_cast<XplodifyPlaylist *>(userdata);

    if(plptr->m_playlist != pl) {
        return NULL;
    }

    return plptr;
}


void XplodifyPlaylist::tracks_added(
        sp_track *const *tracks, int num_tracks, 
        int position) {
    if(m_session) {
        return;
    }

    track_cache_by_rand& tr_cache_rand = m_track_cache.get<0>();
    track_cache_by_rand::const_iterator cit = tr_cache_rand.begin();

    //Fast forward
    cit = cit+position-1;

    for(int i=0 ; i<num_tracks ; i++) {
        boost::shared_ptr<XplodifyTrack> tr(new XplodifyTrack(m_session));
        if(tr->load(tracks[i])){
            tr_cache_rand.insert(cit, track_entry(tr->get_name(), tr));
            cit++;
        }
    }

    return;
}
void XplodifyPlaylist::tracks_removed(const int *tracks, int num_tracks) {

    //Assuming *tracks is ordered : CHECK THIS!
    for(int i=0 ; i<num_tracks ; i++) {
        boost::shared_ptr<XplodifyTrack> tr = 
            remove_track_from_cache(tracks[num_tracks-i-1]);
        tr.reset();
    }

    return;
}
void XplodifyPlaylist::tracks_moved(const int *tracks, int num_tracks, int new_position) {
    //TODO
    return;
}
void XplodifyPlaylist::playlist_renamed() {
    //TODO
    return;
}
void XplodifyPlaylist::playlist_state_changed(){

    //Has the state changed cause we're done loading?
#ifdef _DEBUG
    std::cout << "Playlist state changed" << std::endl;
#endif
    if( m_loading && sp_playlist_is_loaded(m_playlist))
    {
        m_loading = false;
        load_tracks();
#ifdef _DEBUG
        std::cout << "Playlist " << get_name() << " completed loading." << std::endl;
#endif
    }

    return;
}
void XplodifyPlaylist::playlist_update_in_progress(bool done){
    //TODO
    return;
}
void XplodifyPlaylist::playlist_metadata_updated(){
    //TODO
    return;
}
void XplodifyPlaylist::track_created_changed(int position, sp_user *user, int when){
    //TODO
    return;
}
void XplodifyPlaylist::track_seen_changed(int position, bool seen){
    //TODO
    return;
}
void XplodifyPlaylist::description_changed(const char *desc){
    //TODO
    return;
}
void XplodifyPlaylist::image_changed(const byte *image){
    //TODO
    return;
}
void XplodifyPlaylist::track_message_changed(int position, const char *message){
    //TODO
    return;
}
void XplodifyPlaylist::subscribers_changed(){
    //TODO
    return;
}


void SP_CALLCONV XplodifyPlaylist::cb_tracks_added(
        sp_playlist *pl, sp_track *const *tracks, int num_tracks, 
        int position, void *userdata) {
    XplodifyPlaylist * xpl = XplodifyPlaylist::get_playlist_from_udata(pl, userdata);
    if(!xpl) {
        return;
    }

    xpl->tracks_added(tracks, num_tracks, position);
    return;
}
void SP_CALLCONV XplodifyPlaylist::cb_tracks_removed(
        sp_playlist *pl, const int *tracks, int num_tracks, void *userdata) {
    XplodifyPlaylist * xpl = XplodifyPlaylist::get_playlist_from_udata(pl, userdata);

    xpl->tracks_removed(tracks, num_tracks);
    return;
}
void SP_CALLCONV XplodifyPlaylist::cb_tracks_moved(
        sp_playlist *pl, const int *tracks, int num_tracks, 
        int new_position, void *userdata) {
    XplodifyPlaylist * xpl = XplodifyPlaylist::get_playlist_from_udata(pl, userdata);
    if(!xpl) {
        return;
    }

    xpl->tracks_removed(tracks, num_tracks);
    return;
}
void SP_CALLCONV XplodifyPlaylist::cb_playlist_renamed(
        sp_playlist *pl, void *userdata) {
    XplodifyPlaylist * xpl = XplodifyPlaylist::get_playlist_from_udata(pl, userdata);
    if(!xpl) {
        return;
    }

    xpl->playlist_renamed();
    return;
}
void SP_CALLCONV XplodifyPlaylist::cb_playlist_state_changed(
        sp_playlist *pl, void *userdata) {
    XplodifyPlaylist * xpl = XplodifyPlaylist::get_playlist_from_udata(pl, userdata);
    if(!xpl) {
        return;
    }

    xpl->playlist_state_changed();
    return;
}
void SP_CALLCONV XplodifyPlaylist::cb_playlist_update_in_progress(
        sp_playlist *pl, bool done, void *userdata) {
    XplodifyPlaylist * xpl = XplodifyPlaylist::get_playlist_from_udata(pl, userdata);
    if(!xpl) {
        return;
    }

    xpl->playlist_update_in_progress(done);
    return;
}
void SP_CALLCONV XplodifyPlaylist::cb_playlist_metadata_updated(
        sp_playlist *pl, void *userdata) {
    XplodifyPlaylist * xpl = XplodifyPlaylist::get_playlist_from_udata(pl, userdata);
    if(!xpl) {
        return;
    }

    xpl->playlist_metadata_updated();
    return;
}
void SP_CALLCONV XplodifyPlaylist::cb_track_created_changed(
        sp_playlist *pl, int position, sp_user *user, int when, void *userdata) {
    XplodifyPlaylist * xpl = XplodifyPlaylist::get_playlist_from_udata(pl, userdata);
    if(!xpl) {
        return;
    }

    xpl->track_created_changed(position, user, when);
    return;
}
void SP_CALLCONV XplodifyPlaylist::cb_track_seen_changed(
        sp_playlist *pl, int position, bool seen, void *userdata) {
    XplodifyPlaylist * xpl = XplodifyPlaylist::get_playlist_from_udata(pl, userdata);
    if(!xpl) {
        return;
    }

    xpl->track_seen_changed(position, seen);
    return;
}
void SP_CALLCONV XplodifyPlaylist::cb_description_changed(
        sp_playlist *pl, const char *desc, void *userdata) {
    XplodifyPlaylist * xpl = XplodifyPlaylist::get_playlist_from_udata(pl, userdata);
    if(!xpl) {
        return;
    }

    xpl->description_changed(desc);
    return;
}
void SP_CALLCONV XplodifyPlaylist::cb_image_changed(
        sp_playlist *pl, const byte *image, void *userdata) {
    XplodifyPlaylist * xpl = XplodifyPlaylist::get_playlist_from_udata(pl, userdata);
    if(!xpl) {
        return;
    }

    xpl->image_changed(image);
    return;
}
void SP_CALLCONV XplodifyPlaylist::cb_track_message_changed(
        sp_playlist *pl, int position, const char *message, void *userdata) {
    XplodifyPlaylist * xpl = XplodifyPlaylist::get_playlist_from_udata(pl, userdata);
    if(!xpl) {
        return;
    }

    xpl->track_message_changed(position, message);
    return;
}
void SP_CALLCONV XplodifyPlaylist::cb_subscribers_changed(
        sp_playlist *pl, void *userdata){
    XplodifyPlaylist * xpl = XplodifyPlaylist::get_playlist_from_udata(pl, userdata);
    if(!xpl) {
        return;
    }

    xpl->subscribers_changed();
    return;
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
}

XplodifyPlaylistContainer::~XplodifyPlaylistContainer(){
    //TODO: make sure we cleanup properly.
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
        //flush the tracks from the playlist as well...
        it = c_r.erase(it);

        pl->flush();
        pl.reset();
    }
}

void XplodifyPlaylistContainer::playlist_added(sp_playlist *pl, int pos){
    //log this.

    boost::shared_ptr<XplodifyPlaylist> npl(new XplodifyPlaylist(m_session));
    npl->load(pl);
    if(!npl->is_loaded()) {
        m_loading_cache.push_back(npl);
#ifdef _DEBUG
        std::cout << "Playlist loading... Deferring cache insertion." << std::endl;
#endif
    } else {
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
        boost::shared_ptr<XplodifyPlaylist> npl(new XplodifyPlaylist(m_session));
        npl->load(p);
        if(!npl->is_loaded()) {
#ifdef _DEBUG
            std::cout << "Playlist loading... Deferring cache insertion." << std::endl;
#endif
            m_loading_cache.push_back(npl);
        }
        else {
#ifdef _DEBUG
            std::cout << "Playlist " << npl->get_name() << " loaded into plc." << std::endl;
#endif
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
