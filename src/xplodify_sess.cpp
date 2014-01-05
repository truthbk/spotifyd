//Boost.
#include <boost/algorithm/string.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <string>

#include "Spotify.h"

#include "spotify_cust.h"
#include "xplodify_plc.h"
#include "xplodify_pl.h"
#include "xplodify_sess.h"


extern "C" {
#include <libspotify/api.h>
}


XplodifySession::XplodifySession() 
    : m_session(NULL)
    , m_handler(NULL)
    , m_logged_in(false)
    , m_logging_in(false)
    , m_logging_out(false)
    , m_playback_done(1)
    , m_remove_tracks(0)
    , m_mode(SpotifyCmd::LINEAR)
    , m_playback(SpotifyCmd::PAUSE)
    , m_ts(0)
{
    srand(time(NULL));
}

XplodifySession::XplodifySession(XplodifyHandler * h) 
    : m_session(NULL)
    , m_handler(h)
    , m_logged_in(false)
    , m_logging_out(false)
    , m_playback_done(1)
    , m_remove_tracks(0)
    , m_mode(SpotifyCmd::LINEAR)
    , m_playback(SpotifyCmd::PAUSE)
    , m_ts(0)
{
    srand(time(NULL));
}

XplodifySession::~XplodifySession()
{
    //empty
#ifdef _DEBUG
    std::cout << "Xplodify Session destroyed...." << std::endl;
#endif
}


XplodifySession * XplodifySession::get_session_from_udata(sp_session * sp) {
    XplodifySession * s = 
        reinterpret_cast<XplodifySession *>(sp_session_userdata(sp));

    return s;
}


int XplodifySession::init_session(const uint8_t * appkey, size_t appkey_size) {

    sp_error err;

    m_spconfig.api_version = SPOTIFY_API_VERSION;
    m_spconfig.cache_location =  m_handler->get_cachedir().c_str();
    m_spconfig.settings_location = m_handler->get_cachedir().c_str();
    m_spconfig.tracefile = 0x0;
    m_spconfig.application_key = appkey;
    m_spconfig.application_key_size = appkey_size; // Set in main()
    m_spconfig.user_agent = "spotifyd";
    m_spconfig.userdata = this; //we'll use this in callbacks

    //get callbacks ready
    memset(&session_callbacks, 0, sizeof(session_callbacks));
    session_callbacks.connection_error = cb_connection_error;
    session_callbacks.logged_in = cb_logged_in;
    session_callbacks.logged_out = cb_logged_out;
    session_callbacks.notify_main_thread = cb_notify_main_thread;
    session_callbacks.music_delivery = cb_music_delivery;
    session_callbacks.metadata_updated = cb_metadata_updated;
    session_callbacks.play_token_lost = cb_play_token_lost;
    session_callbacks.message_to_user = cb_log_msg;
    session_callbacks.log_message = cb_log_msg;
    session_callbacks.end_of_track = cb_end_of_track;
    session_callbacks.start_playback = cb_start_playback;
    session_callbacks.stop_playback = cb_stop_playback;
    session_callbacks.streaming_error = cb_streaming_error;
    session_callbacks.get_audio_buffer_stats = cb_get_audio_buffer_stats;
    session_callbacks.userinfo_updated = cb_userinfo_updated;

    m_spconfig.callbacks = &session_callbacks;

    err = sp_session_create( &m_spconfig, &m_session );
    if( err != SP_ERROR_OK) {
        return -1;
    }

    //check errors, blah....
    return 0;
}
int XplodifySession::init_session(const uint8_t * appkey, size_t appkey_size, 
        sp_session_callbacks * sess_cb){
    sp_error err;

    m_spconfig.api_version = SPOTIFY_API_VERSION;
    m_spconfig.cache_location =  m_handler->get_cachedir().c_str();
    m_spconfig.settings_location = m_handler->get_cachedir().c_str();
    m_spconfig.tracefile = 0x0;
    m_spconfig.application_key = appkey;
    m_spconfig.application_key_size = appkey_size; // Set in main()
    m_spconfig.user_agent = "spotifyd";
    m_spconfig.userdata = this; //we'll use this in callbacks

    //get callbacks ready
    memcpy(&session_callbacks, sess_cb, sizeof(session_callbacks));

    m_spconfig.callbacks = &session_callbacks;

    err = sp_session_create( &m_spconfig, &m_session );
    if( err != SP_ERROR_OK) {
        return -1;
    }

    //check errors, blah....
    return 0;
}

bool XplodifySession::available(void) {
    bool available;
    boost::mutex::scoped_lock scoped_lock(m_mutex);
    available = (m_active_user.empty() && !m_logged_in);


    return available;
}

void XplodifySession::login( const std::string& username
                          , const std::string& passwd
                          , bool remember ) {

    boost::mutex::scoped_lock scoped_lock(m_mutex);

    //gotta add blob support (see libspotify api).
    sp_error err;
    err = sp_session_login( 
            m_session, 
            username.c_str(), 
            passwd.c_str(), remember, NULL);

    m_active_user = username;

    return;
}

bool XplodifySession::get_logged_in(std::string username) {
    bool logged = false;

    boost::mutex::scoped_lock scoped_lock(m_mutex);
    logged = (user_exists(username) ? m_statuses[username].m_logged_in : false);

    return logged;
}

bool XplodifySession::logout(std::string user, bool unload, bool doflush) {
    sp_error err;

    boost::mutex::scoped_lock scoped_lock(m_mutex);
    if(!is_user_active() || m_active_user != user) {
        return false;
    }
    if(unload) {
        if(m_statuses[m_active_user].m_plcontainer) {
            m_statuses[m_active_user].m_plcontainer->unload(true);
        }
    }

    if(doflush) {
        flush(m_active_user);
    }

    err = sp_session_logout(get_session());
    if(err != SP_ERROR_OK){
        return false;
    }

    m_logging_out = true;

    return true;
}

void XplodifySession::logged_out() {
    boost::mutex::scoped_lock scoped_lock(m_mutex);

    if(!m_logging_out) {
        return;
    }
#ifdef _DEBUG
    std::cout << "Session logged out succesfully." << std::endl;
#endif
    //sp_session_release(get_session());
    //m_active_session.reset();
    m_logged_in = false;
    m_logging_in = false;
    m_logging_out = false;
    m_active_user = std::string();

    return;
}

void XplodifySession::update_plcontainer(std::string user, bool cascade) {

    boost::mutex::scoped_lock scoped_lock(m_mutex);
    if(!user_exists(user)) {
        return;
    }

    if(!m_session){
        return;
    }

    sp_playlistcontainer* c = sp_session_playlistcontainer( m_session );
    if(!c) {
        return;
    }

    m_statuses[user].m_plcontainer->set_plcontainer(c);
    if(cascade) {
        m_statuses[user].m_plcontainer->update_playlist_ptrs(cascade);
    }

    return;
}

boost::shared_ptr<XplodifyPlaylistContainer> XplodifySession::get_pl_container(std::string user) {

    boost::mutex::scoped_lock scoped_lock(m_mutex);
    if(!user_exists(user)) {
        return boost::shared_ptr<XplodifyPlaylistContainer>();
    }

    std::map<std::string, SessionStatus>::iterator it;
    if((m_statuses[user].m_plcontainer != NULL)) {
        return m_statuses[user].m_plcontainer;
    }

    sp_playlistcontainer* c = sp_session_playlistcontainer( m_session );

    if (NULL == c)
    {
        return boost::shared_ptr<XplodifyPlaylistContainer>();
    }

    XplodifyPlaylistContainer * xplc = 
        new XplodifyPlaylistContainer(*this);

    m_statuses[user].m_plcontainer =  boost::shared_ptr<XplodifyPlaylistContainer>(xplc);;

#ifdef _DEBUG
    std::cout << "Loading playlist container..." << std::endl;
#endif

    m_statuses[user].m_plcontainer->load(c);

    return m_statuses[user].m_plcontainer;
}

boost::shared_ptr<XplodifyPlaylistContainer> XplodifySession::get_pl_container(void) {

    boost::mutex::scoped_lock scoped_lock(m_mutex);
    if((m_statuses[m_active_user].m_plcontainer != NULL)) {
        boost::shared_ptr<XplodifyPlaylistContainer> 
            plc(m_statuses[m_active_user].m_plcontainer);
        return plc;
    }

    return boost::shared_ptr<XplodifyPlaylistContainer>();
}

void XplodifySession::set_active_playlist(std::string user, int idx) {
    boost::shared_ptr<XplodifyPlaylistContainer> pc = get_pl_container(user);
    if(!pc) {
        return;
    }

    boost::mutex::scoped_lock scoped_lock(m_mutex);
    boost::shared_ptr<XplodifyPlaylist> pl = pc->get_playlist(idx);
    m_statuses[user].m_playlist = pl;
}

void XplodifySession::set_active_playlist(std::string user, std::string plname) {
    boost::shared_ptr<XplodifyPlaylistContainer> pc = get_pl_container(user);
    if(!pc) {
        return;
    }

    boost::mutex::scoped_lock scoped_lock(m_mutex);
    boost::shared_ptr<XplodifyPlaylist> pl = pc->get_playlist(plname);
    m_statuses[user].m_playlist = pl;
}

std::string XplodifySession::get_playlist_name(void) {
}

std::string XplodifySession::get_playlist_name(std::string user) {

    boost::mutex::scoped_lock scoped_lock(m_mutex);

    std::map<std::string, SessionStatus>::iterator it;
    it = m_statuses.find(user);
    if(it != m_statuses.end()) {
        return it->second.m_playlist->get_name();
    }

    return std::string("");
}

boost::shared_ptr<XplodifyTrack> XplodifySession::get_track(std::string user){

    boost::mutex::scoped_lock scoped_lock(m_mutex);
    if (user_exists(user)) {
        return boost::shared_ptr<XplodifyTrack>();
    }

    return m_statuses[user].m_track;
}

boost::shared_ptr<XplodifyTrack> XplodifySession::get_track(void){
    return get_track(m_active_user);
}

void XplodifySession::update_state_ts(void) {
    boost::mutex::scoped_lock scoped_lock(m_mutex);
    m_ts = std::time(NULL);

    return;
}

int64_t XplodifySession::get_state_ts(void) {
    int64_t state = 0;

    boost::mutex::scoped_lock scoped_lock(m_mutex);
    state = m_ts;

    return state;
}

int64_t XplodifySession::get_state_ts(std::string user) {
    int64_t state = 0;

    boost::mutex::scoped_lock scoped_lock(m_mutex);
    if(!user_exists(user)) {
        return false;
    }

    state = m_statuses[user].m_ts;

    return state;
}

void XplodifySession::set_mode(SpotifyCmd::type mode) {

    boost::mutex::scoped_lock scoped_lock(m_mutex);
    m_mode = mode;
}

SpotifyCmd::type XplodifySession::get_mode(void) {

    boost::mutex::scoped_lock scoped_lock(m_mutex);
    return m_mode;
}

sp_session * XplodifySession::get_sp_session() {

    boost::mutex::scoped_lock scoped_lock(m_mutex);
    return m_session;
}

// Using random access to map as opposed to an iterator for readability...
// This is a performance issue though.
void XplodifySession::set_track(std::string user, int idx) {

    boost::mutex::scoped_lock scoped_lock(m_mutex);
    if(!user_exists(user)) {
        return;
    }

    if( idx < 0 ) {
        m_statuses[user].m_track_idx = NO_TRACK_IDX;
        return;
    }

    boost::shared_ptr<XplodifyTrack> track = m_statuses[user].m_playlist->get_track_at(idx);
    //track has been changed.
    //got to make sure the session has user std::string user logged in before doing this...
    if(m_statuses[user].m_track && (track != m_statuses[user].m_track)) {
        sp_session_player_unload(m_session);
        //m_handler->audio_fifo_flush_now();
        m_statuses[user].m_track_idx = NO_TRACK_IDX;
        m_statuses[user].m_track = boost::shared_ptr<XplodifyTrack>();
    }
    if(!track || track->get_track_error() != SP_ERROR_OK ) {
        return;
    }
    if(m_statuses[user].m_track == track) {
        return;
    }

    m_statuses[user].m_track = track;
    m_statuses[user].m_track_idx = idx;

    sp_session_player_load(m_session, m_statuses[user].m_track->m_track);
#ifdef _DEBUG
    std::cout << "Track " << m_statuses[user].m_track->get_name() << " loaded succesfully." << std::endl;
#endif
}

// Using random access to map as opposed to an iterator for readability...
// This is a performance issue though.
void XplodifySession::set_track(std::string user, std::string trackname) {

    boost::mutex::scoped_lock scoped_lock(m_mutex);
    if( trackname.empty() ) {
        return;
    }

    if(!user_exists(user)) {
        return;
    }


    std::map<std::string, SessionStatus>::iterator it;
    it = m_statuses.find(user);
    if(it == m_statuses.end() || !(it->second.m_playlist)) {
        return;
    }

    boost::shared_ptr<XplodifyTrack> track = it->second.m_playlist->get_track(trackname);
    //track has been changed.
    if(it->second.m_track && (track != it->second.m_track)) {
        sp_session_player_unload(m_session);
        //m_handler->audio_fifo_flush_now();
        m_statuses[user].m_track_idx = NO_TRACK_IDX;
        m_statuses[user].m_track = boost::shared_ptr<XplodifyTrack>();
    }
    if(!track || track->get_track_error() != SP_ERROR_OK ) {
        return;
    }
    if(it->second.m_track == track) {
        return;
    }

    m_statuses[user].m_track = track;
    sp_session_player_load(m_session, m_statuses[user].m_track->m_track);
#ifdef _DEBUG
    std::cout << "Track " << m_statuses[user].m_track->get_name() << " loaded succesfully." << std::endl;
#endif
}

#if 0
void selectPlaylist(const SpotifyCredential& cred, const std::string& playlist) {
    // Your implementation goes here
    printf("selectPlaylist\n");
}
#endif


void XplodifySession::end_of_track() {

    int next;
    boost::shared_ptr<XplodifyTrack> trk;

    boost::mutex::scoped_lock scoped_lock(m_mutex);

    int num = m_statuses[m_active_user].m_playlist->get_num_tracks();

    set_playback_done(1);
    m_handler->set_playback_done(m_playback_done);


#ifdef _DEBUG
    std::cout << "Track has finished." << std::endl;
#endif
    switch(m_mode){
        case SpotifyCmd::REPEAT_ONE:
        case SpotifyCmd::REPEAT:
            break;
        case SpotifyCmd::RAND:
            //Any track on the playlist.
            next = rand() % num + 1;
            scoped_lock.unlock(); //manual unlock -  set_track also will try to hold lock
            set_track(m_active_user, next);
            start_playback();
            break;
        case SpotifyCmd::LINEAR:
            //MEANT to get the NEXT playlist.
            trk = m_statuses[m_active_user].m_playlist->get_next_track();
            scoped_lock.unlock(); //manual unlock -  set_track also will try to hold lock
            set_track(m_active_user, trk->get_name());
            start_playback();
            break;
        default:
            break;
    }

#if 0
    boost::mutex::scoped_lock scoped_lock(m_mutex);
    m_playback_done = 1;
    //h->setPlaybackState(DONE);
    switchSession();
    cond_signal();
    
#endif
}

void XplodifySession::play_token_lost()
{
#if 0
    audio_fifo_flush(&m_audiofifo);

    //Find the session and stop it.
    boost::shared_ptr<XplodifySession> spsession = getActiveSession();

    if (spsession->gettrack() != NULL)
    {
	// unload the session that caused the token loss.
	sp_session_player_unload(sess);
	spsession->settrack(NO_TRACK_IDX);
    }
    switchSession();
#endif
}

//TODO: could have a race-condition here, watch out.
void XplodifySession::logged_in(sp_session *sess, sp_error error) {
    //We've logged in succesfully, lets load pl container, and pl's
    boost::mutex::scoped_lock scoped_lock(m_mutex);

    if(!m_statuses[m_active_user].m_logged_earlier) {
        m_statuses[m_active_user].m_logged_earlier = true;
        scoped_lock.unlock();
        get_pl_container(m_active_user); 
        scoped_lock.lock();
    } else {
        scoped_lock.unlock();
        update_plcontainer(m_active_user, true);
        scoped_lock.lock();
    }

    m_statuses[m_active_user].m_logged_in = true;
    m_logged_in = true;

#ifdef _DEBUG
    std::cout << "Session logged in succesfully." << std::endl;
#endif

    return;
}

void XplodifySession::flush(std::string const user) {
    boost::shared_ptr<XplodifyPlaylistContainer> pc = get_pl_container(user);
    if(!pc) {
        return;
    }

    pc->flush();
    m_statuses[user].m_plcontainer.reset();
#ifdef _DEBUG
    std::cout << "Playlist Container use count: " << pc.use_count() << std::endl;
#endif
}

//Should be called holding lock.
//Doesn't make sense to lock in here because we should ensure the active user 
//doesn't change at the caller.
bool XplodifySession::is_user_active() {

    boost::mutex::scoped_lock scoped_lock(m_mutex);

    return !(m_active_user.empty());
}

//Should be called holding lock.
//Doesn't make sense to lock in here because we should ensure the active user 
//doesn't change at the caller.
bool XplodifySession::user_exists(std::string const user) {

    std::map<std::string, SessionStatus>::iterator it;
    it = m_statuses.find(user);

    return !(it == m_statuses.end());
}

void XplodifySession::start_playback()
{
#ifdef _DEBUG
    std::cout << "Starting playback." << std::endl;
#endif
    boost::mutex::scoped_lock scoped_lock(m_mutex);
    sp_session_player_play(m_session, 1);

    return;
}

void XplodifySession::stop_playback()
{
#ifdef _DEBUG
    std::cout << "Stopping playback." << std::endl;
#endif
    boost::mutex::scoped_lock scoped_lock(m_mutex);
    sp_session_player_play(m_session, 0);
    
    return;
}

void XplodifySession::get_audio_buffer_stats(sp_audio_buffer_stats * stats) {
    m_handler->audio_fifo_stats(stats);
    return;
}

void XplodifySession::userinfo_updated()
{
#if 0
    audio_fifo_flush(&m_audiofifo);

    //Find the session and stop it.
    boost::shared_ptr<XplodifySession> spsession = getActiveSession();

    if (spsession->gettrack() != NULL)
    {
	// unload the session that caused the token loss.
	sp_session_player_unload(sess);
	spsession->settrack(NO_TRACK_IDX);
    }
    switchSession();
#endif
}

int XplodifySession::music_delivery(sp_session *sess, const sp_audioformat *format,
	const void *frames, int num_frames)
{

    int n_frames=0;
    n_frames = m_handler->music_playback(format, frames, num_frames);
    return n_frames;
}

void XplodifySession::notify_main_thread(sp_session *sess)
{

    m_handler->notify_main_thread();
}


/* --------------------------  SESSION CALLBACKS  ------------------------- */
void SP_CALLCONV XplodifySession::cb_logged_in(
        sp_session *sess, sp_error error) {

    XplodifySession * s = XplodifySession::get_session_from_udata(sess);
    if(!s) {
        return;
    }

    s->logged_in(sess, error);
    return;
}

void SP_CALLCONV XplodifySession::cb_logged_out(sp_session *sess) {

    //TODO
    XplodifySession * s = XplodifySession::get_session_from_udata(sess);
    if(!s) {
        return;
    }
    s->logged_out();
    return;
}

void SP_CALLCONV XplodifySession::cb_metadata_updated(sp_session *sess) {

    //TODO
    XplodifySession * s = XplodifySession::get_session_from_udata(sess);
    if(!s) {
        return;
    }
}

void SP_CALLCONV XplodifySession::cb_connection_error(
        sp_session *sess, sp_error error) {

    //TODO
    XplodifySession * s = XplodifySession::get_session_from_udata(sess);
    if(!s) {
        return;
    }
}

void SP_CALLCONV XplodifySession::cb_streaming_error(
        sp_session *sess, sp_error error) {

    //TODO
    XplodifySession * s = XplodifySession::get_session_from_udata(sess);
    if(!s) {
        return;
    }
}

void SP_CALLCONV XplodifySession::cb_msg_to_user(
        sp_session *sess, const char * message) {

    //TODO
    XplodifySession * s = XplodifySession::get_session_from_udata(sess);
    if(!s) {
        return;
    }
}

void SP_CALLCONV XplodifySession::cb_log_msg(
        sp_session *sess, const char * data) {

    //TODO
    XplodifySession * s = XplodifySession::get_session_from_udata(sess);
    if(!s) {
        return;
    }
}

void SP_CALLCONV XplodifySession::cb_notify_main_thread(sp_session *sess)
{

    XplodifySession * s = XplodifySession::get_session_from_udata(sess);
    if(!s) {
        return;
    }

    s->notify_main_thread(sess);
    return;
}

int SP_CALLCONV XplodifySession::cb_music_delivery(
        sp_session *sess, const sp_audioformat *format,
        const void *frames, int num_frames)
{
    int frames_consumed = 0;

    XplodifySession * s = XplodifySession::get_session_from_udata(sess);
    if(!s) {
        return 0;
    }

    frames_consumed = 
        s->music_delivery(sess, format, frames, num_frames);
    return frames_consumed; //or whatever...
}

void SP_CALLCONV XplodifySession::cb_play_token_lost(sp_session *sess) {

    XplodifySession * s = XplodifySession::get_session_from_udata(sess);
    if(!s) {
        return;
    }

    s->play_token_lost();
    return;
}

void SP_CALLCONV XplodifySession::cb_end_of_track(sp_session * sess) {
    //sometimes takes for ever to get called by libspotify. deprecating it...
#if 0
    XplodifySession * s = XplodifySession::get_session_from_udata(sess);
    if(!s) {
        return;
    }

    s->end_of_track();
#endif
    return;

}

void SP_CALLCONV XplodifySession::cb_userinfo_updated(sp_session * sp) {
    //TODO
    XplodifySession * s = XplodifySession::get_session_from_udata(sp);
    if(!s) {
        return;
    }
    s->userinfo_updated();
    return;
}
void SP_CALLCONV XplodifySession::cb_start_playback(sp_session * sp) {
    //TODO
    XplodifySession * s = XplodifySession::get_session_from_udata(sp);
    if(!s) {
        return;
    }
    s->start_playback();
    return;
}
void SP_CALLCONV XplodifySession::cb_stop_playback(sp_session * sp) {
    //TODO
    XplodifySession * s = XplodifySession::get_session_from_udata(sp);
    if(!s) {
        return;
    }
    s->stop_playback();
    return;
}
void SP_CALLCONV XplodifySession::cb_get_audio_buffer_stats(sp_session * sp,
        sp_audio_buffer_stats *stats) {

    XplodifySession * s = XplodifySession::get_session_from_udata(sp);
    if(!s) {
        return;
    }
    s->get_audio_buffer_stats(stats);
    return;
}


/* --------------------------  PLAYLIST CALLBACKS  ------------------------- */
#if 0 //PENDING
void SP_CALLCONV XplodifySession::cb_tracks_added(
        sp_playlist *pl, sp_track * const *tracks,
        int num_tracks, int position, void *userdata)
{
    if(!g_handler) {
        return;
    }

    return g_handler->tracks_added(pl, tracks, num_tracks, position, userdata);
}

void SP_CALLCONV XplodifySession::cb_tracks_removed(
        sp_playlist *pl, const int *tracks,
        int num_tracks, void *userdata)
{
    if(!g_handler) {
        return;
    }

    return g_handler->tracks_removed(pl, tracks, num_tracks, userdata);
}

void SP_CALLCONV XplodifySession::cb_tracks_moved(
        sp_playlist *pl, const int *tracks,
        int num_tracks, int new_position, void *userdata)
{
    if(!g_handler) {
        return;
    }

    return g_handler->tracks_moved(pl, tracks, num_tracks, new_position, userdata);
}

void SP_CALLCONV XplodifySession::cb_playlist_renamed(
        sp_playlist *pl, void *userdata)
{
    if(!g_handler) {
        return;
    }

    return g_handler->playlist_renamed(pl, userdata);
}
#endif
