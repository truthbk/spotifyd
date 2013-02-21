#include <cstdint>
#include <cstring>
#include <string>

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
    , m_playlist(NULL){
    //EMPTY
}
XplodifyPlaylist::~XplodifyPlaylist() {
    //EMPTY
}

boost::shared_ptr<XplodifyTrack> 
XplodifyPlaylist::remove_track_from_cache(int idx) {

    track_cache_by_sequence& tr_cache_seqd = m_track_cache.get<0>();
    track_cache_by_sequence::const_iterator cit = tr_cache_seqd.begin();

    for(int i=0 ; i<idx ; i++) {
        cit++;
    }
    track_cache_by_sequence::iterator it = tr_cache_seqd.erase(cit);
    return it->track;
}

boost::shared_ptr<XplodifyTrack> 
XplodifyPlaylist::remove_track_from_cache(std::string& name){
    track_cache_by_name& tr_cache_name = m_track_cache.get<1>();
    track_cache_by_name::iterator it = tr_cache_name.find(name);
    if(it == tr_cache_name.end()) {
        return boost::shared_ptr<XplodifyTrack>();
    }

    tr_cache_name.erase(name);
    return it->track;
}

bool XplodifyPlaylist::load(sp_playlist * pl) {
    if(!pl) {
        return false;
    }

    m_playlist = pl;
    sp_playlist_add_callbacks(pl, const_cast<sp_playlist_callbacks *>(&cbs), this);

    if(!sp_playlist_is_loaded(pl))
    {    m_loading = true;
    } else {
        //load tracks
    }

    return true;
}

bool XplodifyPlaylist::loadTracks() {
    int n;

    if(m_loading) {
        return false;
    }

    n = sp_playlist_num_tracks(m_playlist);
    for(int i=0 ; i<n ; i++) {
        sp_track * t = sp_playlist_track(m_playlist, i);

        track_cache_by_sequence& tr_cache_seqd = m_track_cache.get<0>();

        boost::shared_ptr<XplodifyTrack> tr(new XplodifyTrack(m_session));
        if(tr->load(t)){
            tr_cache_seqd.push_back(track_entry(tr->get_name(), tr));
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
std::string XplodifyPlaylist::getName() {
    if(!m_playlist) {
        return std::string("");
    }

    return std::string(sp_playlist_name(m_playlist));
}

XplodifyPlaylist * XplodifyPlaylist::getPlaylistFromUData(
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

    track_cache_by_sequence& tr_cache_seqd = m_track_cache.get<0>();
    track_cache_by_sequence::const_iterator cit = tr_cache_seqd.begin();

    //Fast forward
    for(int i=0 ; i<position ; i++) {
        cit++;
    }

    for(int i=0 ; i<num_tracks ; i++) {
        boost::shared_ptr<XplodifyTrack> tr(new XplodifyTrack(m_session));
        if(tr->load(tracks[i])){
            tr_cache_seqd.insert(cit, track_entry(tr->get_name(), tr));
            cit++;
        }
    }

    return;
}
void XplodifyPlaylist::tracks_removed(const int *tracks, int num_tracks) {

    //Assuming *tracks is ordered : CHECK THIS!
    for(int i=0 ; i<num_tracks ; i++) {
        remove_track_from_cache(tracks[num_tracks-i-1]);
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
    //TODO
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
    XplodifyPlaylist * xpl = XplodifyPlaylist::getPlaylistFromUData(pl, userdata);
    if(!xpl) {
        return;
    }

    xpl->tracks_added(tracks, num_tracks, position);
    return;
}
void SP_CALLCONV XplodifyPlaylist::cb_tracks_removed(
        sp_playlist *pl, const int *tracks, int num_tracks, void *userdata) {
    XplodifyPlaylist * xpl = XplodifyPlaylist::getPlaylistFromUData(pl, userdata);

    xpl->tracks_removed(tracks, num_tracks);
    return;
}
void SP_CALLCONV XplodifyPlaylist::cb_tracks_moved(
        sp_playlist *pl, const int *tracks, int num_tracks, 
        int new_position, void *userdata) {
    XplodifyPlaylist * xpl = XplodifyPlaylist::getPlaylistFromUData(pl, userdata);
    if(!xpl) {
        return;
    }

    xpl->tracks_removed(tracks, num_tracks);
    return;
}
void SP_CALLCONV XplodifyPlaylist::cb_playlist_renamed(
        sp_playlist *pl, void *userdata) {
    XplodifyPlaylist * xpl = XplodifyPlaylist::getPlaylistFromUData(pl, userdata);
    if(!xpl) {
        return;
    }

    xpl->playlist_renamed();
    return;
}
void SP_CALLCONV XplodifyPlaylist::cb_playlist_state_changed(
        sp_playlist *pl, void *userdata) {
    XplodifyPlaylist * xpl = XplodifyPlaylist::getPlaylistFromUData(pl, userdata);
    if(!xpl) {
        return;
    }

    xpl->playlist_state_changed();
    return;
}
void SP_CALLCONV XplodifyPlaylist::cb_playlist_update_in_progress(
        sp_playlist *pl, bool done, void *userdata) {
    XplodifyPlaylist * xpl = XplodifyPlaylist::getPlaylistFromUData(pl, userdata);
    if(!xpl) {
        return;
    }

    xpl->playlist_update_in_progress(done);
    return;
}
void SP_CALLCONV XplodifyPlaylist::cb_playlist_metadata_updated(
        sp_playlist *pl, void *userdata) {
    XplodifyPlaylist * xpl = XplodifyPlaylist::getPlaylistFromUData(pl, userdata);
    if(!xpl) {
        return;
    }

    xpl->playlist_metadata_updated();
    return;
}
void SP_CALLCONV XplodifyPlaylist::cb_track_created_changed(
        sp_playlist *pl, int position, sp_user *user, int when, void *userdata) {
    XplodifyPlaylist * xpl = XplodifyPlaylist::getPlaylistFromUData(pl, userdata);
    if(!xpl) {
        return;
    }

    xpl->track_created_changed(position, user, when);
    return;
}
void SP_CALLCONV XplodifyPlaylist::cb_track_seen_changed(
        sp_playlist *pl, int position, bool seen, void *userdata) {
    XplodifyPlaylist * xpl = XplodifyPlaylist::getPlaylistFromUData(pl, userdata);
    if(!xpl) {
        return;
    }

    xpl->track_seen_changed(position, seen);
    return;
}
void SP_CALLCONV XplodifyPlaylist::cb_description_changed(
        sp_playlist *pl, const char *desc, void *userdata) {
    XplodifyPlaylist * xpl = XplodifyPlaylist::getPlaylistFromUData(pl, userdata);
    if(!xpl) {
        return;
    }

    xpl->description_changed(desc);
    return;
}
void SP_CALLCONV XplodifyPlaylist::cb_image_changed(
        sp_playlist *pl, const byte *image, void *userdata) {
    XplodifyPlaylist * xpl = XplodifyPlaylist::getPlaylistFromUData(pl, userdata);
    if(!xpl) {
        return;
    }

    xpl->image_changed(image);
    return;
}
void SP_CALLCONV XplodifyPlaylist::cb_track_message_changed(
        sp_playlist *pl, int position, const char *message, void *userdata) {
    XplodifyPlaylist * xpl = XplodifyPlaylist::getPlaylistFromUData(pl, userdata);
    if(!xpl) {
        return;
    }

    xpl->track_message_changed(position, message);
    return;
}
void SP_CALLCONV XplodifyPlaylist::cb_subscribers_changed(
        sp_playlist *pl, void *userdata){
    XplodifyPlaylist * xpl = XplodifyPlaylist::getPlaylistFromUData(pl, userdata);
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
    : m_session(sess)
    , m_plcontainer(NULL)
    , m_loading(false) 
{
}

bool XplodifyPlaylistContainer::load(sp_playlistcontainer * plc) {
    if(!plc) {
        return false;
    }

    m_plcontainer = plc;
    sp_playlistcontainer_add_callbacks(plc, 
            const_cast<sp_playlistcontainer_callbacks *>(&cbs), this);

    m_loading = true;

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

void XplodifyPlaylistContainer::addPlaylist(boost::shared_ptr<XplodifyPlaylist> pl) {
    if(!pl) {
        return;
    }

    //do this with exceptions once this is rolling.
    std::string name(pl->getName());
    if(!name.empty()) {
            m_pl_cache.get<1>().insert(pl_entry(name, pl));
    }
}

void XplodifyPlaylistContainer::playlist_added(sp_playlist *pl, int pos){
    //log this.
    return;
}

void XplodifyPlaylistContainer::playlist_removed(sp_playlist *pl, int pos){

    //we use hashed index, as opposed to sequenced one: faster.
    pl_cache_by_name& c = m_pl_cache.get<1>();
    c.erase(std::string(sp_playlist_name(pl)));

}

//might have to lock().
void XplodifyPlaylistContainer::playlist_moved(sp_playlist *pl, int pos, int newpos){

    //put in the right place in the sequenced index...
    pl_cache_by_sequence& c_s = m_pl_cache.get<0>();
    pl_cache_by_name& c_n = m_pl_cache.get<1>();

    pl_cache_by_name::iterator it = c_n.find(sp_playlist_name(pl));

    //shouldn't happen
    if(it == m_pl_cache.get<1>().end() ) {
        return;
    }

    boost::shared_ptr<XplodifyPlaylist> xpl(it->_playlist);
    //remove it from the list
    c_n.erase(it);

    //add it in the new position.
    pl_cache_by_sequence::iterator sit = c_s.begin();
    for(int i=0 ; i<newpos ; sit++ ) {
        //nothing
    }
    c_s.insert(sit, pl_entry(xpl->getName(), xpl));
    return;
}

void XplodifyPlaylistContainer::container_loaded(){
    int n;

    n = sp_playlistcontainer_num_playlists(m_plcontainer);
    for(int i=0 ; i<n ; i++ ) {
        sp_playlist * p = sp_playlistcontainer_playlist( m_plcontainer, i);
        boost::shared_ptr<XplodifyPlaylist> npl(new XplodifyPlaylist(m_session));
        npl->load(p);

        addPlaylist(npl);
    }
}


XplodifyPlaylistContainer * XplodifyPlaylistContainer::getPlaylistContainerFromUData(
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
        XplodifyPlaylistContainer::getPlaylistContainerFromUData(pc, userdata);
    if(!xplc) {
        return;
    }

    xplc->playlist_added(pl, pos);
}

void SP_CALLCONV XplodifyPlaylistContainer::cb_playlist_removed(
        sp_playlistcontainer *pc, sp_playlist *pl, int pos, void *userdata) {

    XplodifyPlaylistContainer * xplc = 
        XplodifyPlaylistContainer::getPlaylistContainerFromUData(pc, userdata);
    if(!xplc) {
        return;
    }

    xplc->playlist_removed(pl, pos);
}

void SP_CALLCONV XplodifyPlaylistContainer::cb_playlist_moved(
        sp_playlistcontainer *pc, sp_playlist *pl, int pos, int newpos, void *userdata){

    XplodifyPlaylistContainer * xplc = 
        XplodifyPlaylistContainer::getPlaylistContainerFromUData(pc, userdata);
    if(!xplc) {
        return;
    }

    xplc->playlist_moved(pl, pos, newpos);
}

void SP_CALLCONV XplodifyPlaylistContainer::cb_container_loaded(
        sp_playlistcontainer * pc, void *userdata){

    XplodifyPlaylistContainer * xplc = 
        XplodifyPlaylistContainer::getPlaylistContainerFromUData(pc, userdata);
    if(!xplc) {
        return;
    }

    xplc->container_loaded();
}