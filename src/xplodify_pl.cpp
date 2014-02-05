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
#include "xplodify_plc.h"

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

XplodifyPlaylist::XplodifyPlaylist(XplodifySession& sess, int idx) 
    : Lockable()
    , Cacheable()
    , m_session(sess)
    , m_playlist(NULL)
    , m_loading(true) 
    , m_name()
    , m_index(idx)
    , m_num_tracks(){
    //EMPTY
}
XplodifyPlaylist::~XplodifyPlaylist() {
    //EMPTY
#ifdef _DEBUG
    std::cout << "XplodifyPlaylist being destroyed..." << std::endl;
#endif
    lock();
    if(m_playlist){
        sp_playlist_remove_callbacks(m_playlist, const_cast<sp_playlist_callbacks *>(&cbs), this);
        //sp_playlist_release(m_playlist);
        m_playlist = NULL;
    }
    unlock();
}

boost::shared_ptr<XplodifyTrack> 
XplodifyPlaylist::remove_track_from_cache(int idx) {

    track_cache_by_rand& tr_cache_rand = m_track_cache.get<0>();

    if(tr_cache_rand.size() < static_cast<unsigned long>(idx-1)) {
        return boost::shared_ptr<XplodifyTrack>();
    }

    track_cache_by_rand::const_iterator cit = tr_cache_rand.begin();

    cit = cit+idx-1;
    boost::shared_ptr<XplodifyTrack> ret_track(cit->track);

    tr_cache_rand.erase(cit);

    m_session.update_state_ts();
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

    m_session.update_state_ts();
    return ret_track;
}

bool XplodifyPlaylist::load(sp_playlist * pl) {
    if(!pl) {
        return false;
    }

    set_sp_playlist(pl);
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

    lock();
    track_cache_by_rand& t_r = m_track_cache.get<0>();
    std::size_t sz = t_r.size();

    for(std::size_t i=0 ; i<sz ; i++) {
        boost::shared_ptr<XplodifyTrack> tr = get_track(0, true);
        tr.reset();
    }

    m_loading = false;
    unlock();

    m_session.update_state_ts();
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
    lock();
    for(int i=0 ; i<n ; i++) {
        sp_track * t = sp_playlist_track(m_playlist, i);

        track_cache_by_rand& tr_cache_rand = m_track_cache.get<0>();
        std::pair<track_r_iterator, bool> p;

        boost::shared_ptr<XplodifyTrack> tr(new XplodifyTrack(m_session));
        if(tr->load(t, i)){
            tr->cache();
            std::string trname(tr->get_name());
            track_entry tr_entry(trname, tr);
            p = tr_cache_rand.push_back(tr_entry);
            if(p.second) {
#ifdef _DEBUG
                std::cout << "Track " << tr->get_name() << " loaded for playlist "
                    << get_name() << std::endl;
#endif
            } else {
                m_pending_tracks.push_back(tr);
            }
        } else {
            m_pending_tracks.push_back(tr);
        }
    }
    m_session.update_state_ts();
    unlock();
    return true;
}

bool XplodifyPlaylist::set_sp_playlist(sp_playlist * pl) {

    if(!pl) {
        return false;
    }
    m_playlist = pl;
    sp_playlist_add_ref(m_playlist);
    sp_error err = sp_playlist_add_callbacks(
            m_playlist, const_cast<sp_playlist_callbacks *>(&cbs), this);

    return true;
}

void XplodifyPlaylist::update_track_ptrs() {

    lock();
    track_cache_by_rand& t_r = m_track_cache.get<0>();
    for(uint32_t i=0 ; i<t_r.size() ; i++) {
        sp_track * t = sp_playlist_track(m_playlist, t_r[i].track->get_index(true));
        t_r[i].track->set_sp_track(t);
    }
    unlock();
    return;
}
bool XplodifyPlaylist::unload(bool cascade) {

    lock();

    if(cascade) {
        track_cache_by_rand& t_r = m_track_cache.get<0>();
        for(uint32_t i=0 ; i<t_r.size() ; i++) {
            t_r[i].track->unload();
        }
    }
    //sp_playlist_release(m_playlist);
    sp_playlist_remove_callbacks(m_playlist, const_cast<sp_playlist_callbacks *>(&cbs), this);

    //m_track_cache.get<0>().clear();
    m_loading = false;
    m_playlist = NULL;
    unlock();

    return true;
}

//when we put in Exceptions this will be a lot cleaner.
std::string XplodifyPlaylist::get_name(bool cache) {
    if(!m_playlist && !is_cached()) {
        return std::string("");
    }
    if(cache && is_cached()) {
        return m_name;
    }

    return std::string(sp_playlist_name(m_playlist));
}

int XplodifyPlaylist::get_index(bool cache) {
    if(cache) {
        return m_index;
    }

    return m_index; //can't find right API call right now.

}

size_t XplodifyPlaylist::get_num_tracks(bool cache){
    track_cache_by_rand& t_r = m_track_cache.get<0>();
    if(cache && is_cached()) {
        return m_num_tracks;
    }

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
    m_session.update_state_ts();
}

void XplodifyPlaylist::add_track(boost::shared_ptr<XplodifyTrack> tr, int pos) {
    if(!tr) {
        return;
    }

    //get iterator...
    track_cache_by_rand& t_r = m_track_cache.get<0>();
    track_cache_by_rand::iterator it = t_r.iterator_to(t_r[pos]);

    t_r.insert(it, track_entry(tr->get_name(), tr));
    m_session.update_state_ts();
}

boost::shared_ptr<XplodifyTrack> XplodifyPlaylist::get_track(int pos, bool remove) {
    boost::shared_ptr<XplodifyTrack> t;

    track_cache_by_rand& t_r = m_track_cache.get<0>();
    track_cache_by_name& t_n = m_track_cache.get<1>();
    track_cache_by_rand::iterator it = t_r.iterator_to(t_r[pos]);

    if(it == t_r.end()) {
        t = boost::shared_ptr<XplodifyTrack>();
    } else {
        track_cache_by_name::iterator itn = t_n.find(it->track->get_name());
        t = it->track;
        m_it_name = itn;
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
    track_cache_by_name::iterator it(t_n.find(name));

    if(it == t_n.end()) {
        t = boost::shared_ptr<XplodifyTrack>();
    } else {
        t = it->track;
        m_it_name = it;
    }
    if(remove) {
        t_n.erase(it);
    }

    return t;
}

boost::shared_ptr<XplodifyTrack> XplodifyPlaylist::get_next_track() {
    track_cache_by_name& t_n = m_track_cache.get<1>();

    track_cache_by_name::iterator it(m_it_name);
    it++;
    if(it == t_n.end()) {
        it = t_n.begin();
    }
    return it->track;
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
        sp_track * const *tracks, int num_tracks, 
        int position) {
#if 0
    boost::shared_ptr<XplodifySession> sess(m_session);
    if(!sess) {
        return;
    }

    track_cache_by_rand& tr_cache_rand = m_track_cache.get<0>();
    track_cache_by_rand::const_iterator cit = tr_cache_rand.begin();

    //Fast forward
    cit = cit+position-1;

    for(int i=0 ; i<num_tracks ; i++) {
        boost::shared_ptr<XplodifyTrack> tr(new XplodifyTrack(sess));
        std::pair<track_r_iterator, bool> p;
        if(tr->load(tracks[i])){
            std::string trname(tr->get_name());
            if(!trname.empty()) {
                p = tr_cache_rand.insert(cit, track_entry(trname, tr));
                if(p.second) {
                    cit++;
                }
            }
        }
    }

    sess->update_state_ts();
#endif
    return;
}
void XplodifyPlaylist::tracks_removed(const int *tracks, int num_tracks) {


    //Assuming *tracks is ordered : CHECK THIS!
    for(int i=0 ; i<num_tracks ; i++) {
        boost::shared_ptr<XplodifyTrack> tr = 
            remove_track_from_cache(tracks[num_tracks-i-1]);
        if(tr != NULL) {
            tr.reset();
        }
    }

    m_session.update_state_ts();
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
    if(sp_playlist_is_loaded(m_playlist))
    {
        if(m_loading) {
            m_loading = false;
            //we should now create shared_ptr and insert into cache.
            m_session.get_pl_container()->add_playlist(shared_from_this());
            m_session.get_pl_container()->update_cache();
        }
        load_tracks();
#ifdef _DEBUG
        std::cout << "Playlist " << get_name() << " completed loading." << std::endl;
#endif
    }

    m_session.update_state_ts();
    return;
}
void XplodifyPlaylist::playlist_update_in_progress(bool done){
    //TODO
    return;
}
void XplodifyPlaylist::playlist_metadata_updated(){
    //If playlist metadata has been updated, we first check
    //for pending tracks that are ready....
    track_cache_by_rand& tr_cache_rand = m_track_cache.get<0>();

    typedef std::vector< boost::shared_ptr<XplodifyTrack> > wvec;
    wvec::iterator it;
    lock();
    for (it=m_pending_tracks.begin() ; it != m_pending_tracks.end() ; ) {
        boost::shared_ptr<XplodifyTrack> t = *it;
        std::pair<track_r_iterator, bool> p;

        if(t->is_loaded()) {
            std::string trname(t->get_name());
            track_entry tr_entry(trname, t);
            p = tr_cache_rand.push_back(tr_entry);
            if(p.second) {
                it = m_pending_tracks.erase(it);
#ifdef _DEBUG
                std::cout << "Track " << t->get_name() << " loaded for playlist "
                    << get_name() << std::endl;
#endif
            } else {
                it++;
            }
        } else {
            it++;
        }
    }

    m_session.update_state_ts();
    unlock();
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


void XplodifyPlaylist::cache(void) {

    if (is_cached()) {
        return;
    }

    m_name = get_name();
    m_num_tracks = get_num_tracks();

    m_cached = true;
    return;
}
void XplodifyPlaylist::uncache(void){

    m_cached = false;
    return;
}
bool XplodifyPlaylist::is_cached(void){
    return m_cached;
}
