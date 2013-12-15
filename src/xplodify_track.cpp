#include <cstdint>
#include <cstring>
#include <string>

//boost
#include <boost/shared_ptr.hpp>

#include "Spotify.h"
#include "xplodify_track.h"

extern "C" {
    #include <libspotify/api.h>
}

//TODO: exceptions.

XplodifyTrack::XplodifyTrack(boost::shared_ptr<XplodifySession> sess) 
    : Cacheable()
    , m_sess(sess) 
    , m_track(NULL)
    , m_name()
    , m_index(0)
    , m_duration(0)
    , m_num_artists(0)
    , m_artists()
    , m_disc(0)
    , m_popularity(0)
    , m_starred(0){
        //empty
}

XplodifyTrack::~XplodifyTrack() {
#ifdef _DEBUG
    std::cout << "Destroying Track." << std::endl;
#endif
}


bool XplodifyTrack::load(sp_track * track, int idx){
    m_track = track;
    m_index = idx;
    sp_track_add_ref(m_track);
    return is_loaded();
}

bool XplodifyTrack::set_sp_track(sp_track * track) {
    if(!track){
        return false;
    }
    m_track = track;
    sp_track_add_ref(m_track);

    return true;
}

void XplodifyTrack::unload(){
    sp_track_release(m_track);
    m_track = NULL;
}
bool XplodifyTrack::is_loaded(){
    if(!m_track) {
        return false;
    }
    return sp_track_is_loaded(m_track);
}
bool XplodifyTrack::is_streamable(){
    if(!m_track || !is_loaded()) {
        return false;
    }

    sp_track_availability available;
    boost::shared_ptr<XplodifySession> sess(m_sess);
    available = sp_track_get_availability(sess->get_sp_session(), m_track);
    if(available == SP_TRACK_AVAILABILITY_NOT_STREAMABLE) {
        return false;
    }

    return true;
}
std::string XplodifyTrack::get_name(bool cache){
    if(!m_track) {
        return std::string();
    }
    if(cache && is_cached()) {
        return m_name;
    }
    return std::string(sp_track_name(m_track));
}
int XplodifyTrack::get_index(bool cache){
    if(!m_track) {
        return 0;
    }
    if(cache && is_cached()) {
        return m_index;
    }
    return sp_track_index(m_track);

}
int XplodifyTrack::get_duration(bool cache){
    if(!m_track) {
        return 0;
    }
    if(cache && is_cached()) {
        return m_duration;
    }
    return sp_track_duration(m_track);

}
int XplodifyTrack::get_num_artists(bool cache){
    if(!m_track) {
        return 0;
    }
    if(cache && is_cached()) {
        return m_num_artists;
    }
    return sp_track_num_artists(m_track);
}
//If idx>m_num_artists return first artist.
std::string XplodifyTrack::get_artist(int idx, bool cache){
    if(!m_track) {
        return std::string();
    }
    if(idx >= m_num_artists) {
        idx=0;
    }
    if(cache && is_cached()){
        return m_artists[idx];
    }
    return std::string(sp_artist_name(sp_track_artist(m_track, idx)));
}

bool XplodifyTrack::is_starred(bool cache){
    if(!m_track) {
        return false;
    }
    if(cache && is_cached()) {
        return m_starred;
    }
    boost::shared_ptr<XplodifySession> sess(m_sess);
    return sp_track_is_starred(sess->get_sp_session(), m_track);
}

int XplodifyTrack::get_disc(bool cache){
    if(!m_track) {
        return 0;
    }
    if(cache && is_cached()) {
        return m_disc;
    }
    return sp_track_disc(m_track);
}
int XplodifyTrack::get_popularity(bool cache){
    if(!m_track) {
        return 0;
    }
    if(cache && is_cached()) {
        return m_popularity;
    }
    return sp_track_popularity(m_track);
}
void XplodifyTrack::set_starred(bool star){
    if(!m_track) {
        return;
    }
    boost::shared_ptr<XplodifySession> sess(m_sess);
    sp_track_set_starred(sess->get_sp_session(), &m_track, 1, star);
}

sp_error XplodifyTrack::get_track_error(){
    return sp_track_error(m_track);
}

void XplodifyTrack::cache(void) {

    if (is_cached()) {
        return;
    }
    m_name = get_name(false);
    m_duration = get_duration(false);
    m_num_artists = get_num_artists(false);
    m_disc = get_disc(false);
    m_popularity = get_popularity(false);
    m_starred = is_starred(false);

    for(int i=0 ; i<m_num_artists ; i++) {
        m_artists.push_back(get_artist(i, false));
    }

    m_cached = true;
    return;
}
void XplodifyTrack::uncache(void){

    m_cached = true;
    return;
}
bool XplodifyTrack::is_cached(void){
    return m_cached;
}

