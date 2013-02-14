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
    cb_playlist_metada_updated,
    cb_track_created_changed,
    cb_track_seen_changed,
    cb_description_changed,
    cb_image_changed,
    cb_track_message_changed,
    cb_subscribers_changed
}

XplodifyPlaylist::XplodifyPlaylist(boost::shared_ptr<XplodifySession> sess) 
    : m_session(sess)
    , m_playlist(NULL)
{
    //EMPTY
};

bool XplodifyPlaylist::load(sp_playlist * pl) {
    if(!pl) {
        return false;
    }

    m_playlist = pl;
    sp_playlist_add_callbacks(plc, &cbs);

    m_loading = true;

    return m_loading;
}
bool XplodifyPlaylistContainer::unload() {

    if(!m_plcontainer){
        return true;
    }

    sp_playlistcontainer_remove_callbacks(pl, &cbs);
    m_pl_cache.get<0>().clear();
    m_playlist(NULL);

    return true;
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
    cb_container_loadad
}

XplodifyPlaylistContainer::XplodifyPlaylistContainer(
        boost::shared_ptr<XplodifySession> sess)
    : m_session(sess)
    , m_plcontainer(NULL)
    , loading(false) 
{
}

bool XplodifyPlaylistContainer::load(sp_playlistcontainer * plc) {
    if(!plc) {
        return false;
    }

    m_plcontainer = plc;
    sp_playlistcontainer_add_callbacks(plc, &cbs);

    m_loading = true;

    return m_loading;
}
bool XplodifyPlaylistContainer::unload() {

    if(!m_plcontainer){
        return true;
    }

    sp_playlistcontainer_remove_callbacks(plc, &cbs);
    m_plcontainer(NULL);

    return true;
}

void XplodifyPlaylistContainer::playlist_added(sp_playlist *pl, int pos){
}
void XplodifyPlaylistContainer::playlist_removed(sp_playlist *pl, int pos){
}
void XplodifyPlaylistContainer::playlist_moved(sp_playlist *pl, int pos, int newpos){
}
void XplodifyPlaylistContainer::container_loaded(){
}


XplodifyPlaylistContainer * XplodifyPlaylistContainer::getPlaylistContainerFromUData(
        sp_playlistcontainer * plc, void * userdata) {
    XplodifyPlaylist * plcptr = 
        reinterpret_cast<XplodifyPlaylistContainer *>(userdata);

    if(plcptr->m_playlistcontainer != plc) {
        return NULL;
    }

    return plc;
}


void SP_CALLCONV XplodifyPlaylistContainer::cb_playlist_added(
        sp_playlistcontainer *pc, sp_playlist *pl, int pos, void *userdata) {

    XplodifyPlaylistiContainer * xplc = 
        XplodifyPlaylistContainer::getPlaylistContainerFromUData(pc, userdata);
    if(!xplc) {
        return;
    }

    xplc->playlist_added(pl, pos);
}

void SP_CALLCONV XplodifyPlaylistContainer::cb_playlist_removed(
        sp_playlistcontainer *pc, sp_playlist *pl, int pos, void *userdata) {

    XplodifyPlaylistiContainer * xplc = 
        XplodifyPlaylistContainer::getPlaylistContainerFromUData(pc, userdata);
    if(!xplc) {
        return;
    }

    xplc->playlist_removed(pl, pos);
}

void SP_CALLCONV XplodifyPlaylistContainer::cb_playlist_moved(
        sp_playlistcontainer *pc, sp_playlist *pl, int pos, int newpos, void *userdata){

    XplodifyPlaylistiContainer * xplc = 
        XplodifyPlaylistContainer::getPlaylistContainerFromUData(pc, userdata);
    if(!xplc) {
        return;
    }

    xplc->playlist_moved(pl, posi, newpos);
}

void SP_CALLCONV XplodifyPlaylistContainer::cb_container_loaded(
        sp_playlistcontainer * pc, void *userdata){

    XplodifyPlaylistiContainer * xplc = 
        XplodifyPlaylistContainer::getPlaylistContainerFromUData(pc, userdata);
    if(!xplc) {
        return;
    }

    xplc->container_loaded();
}
