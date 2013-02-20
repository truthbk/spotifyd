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
    : m_sess(sess) {
        //empty
}

XplodifyTrack::~XplodifyTrack() {
}


bool XplodifyTrack::load(sp_track * track){
    m_track = track;
    sp_track_add_ref(m_track);
    return true;
}
void XplodifyTrack::unload(){
    sp_track_release(m_track);
    m_track = NULL;
}
bool XplodifyTrack::isLoading(){
    if(!m_track) {
        return false;
    }
    return sp_track_is_loaded(m_track);
}
std::string XplodifyTrack::get_name(){
    if(!m_track) {
        return std::string();
    }
    return std::string(sp_track_name(m_track));
}
int XplodifyTrack::get_duration(){
    if(!m_track) {
        return 0;
    }
    return sp_track_duration(m_track);

}
int XplodifyTrack::get_num_artists(){
    if(!m_track) {
        return 0;
    }
    return sp_track_num_artists(m_track);
}
std::string XplodifyTrack::getArtist(int idx){
    if(!m_track) {
        return std::string();
    }
    return std::string(sp_artist_name(sp_track_artist(m_track, idx)));
}

bool XplodifyTrack::is_starred(){
    if(!m_track) {
        return false;
    }
    return sp_track_is_starred(m_sess->get_sp_session(), m_track);
}

int XplodifyTrack::get_disc(){
    if(!m_track) {
        return 0;
    }
    return sp_track_disc(m_track);
}
int XplodifyTrack::get_popularity(){
    if(!m_track) {
        return 0;
    }
    return sp_track_popularity(m_track);
}
void XplodifyTrack::set_starred(bool star){
    if(!m_track) {
        return;
    }
    sp_track_set_starred(m_sess->get_sp_session(), &m_track, 1, star);
}
