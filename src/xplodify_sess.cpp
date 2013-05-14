//Boost.
#include <boost/algorithm/string.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include <cstring>
#include <string>

#include "Spotify.h"
#include "spotify_cust.h"
#include "xplodify_pl.h"
#include "xplodify_sess.h"


extern "C" {
#include <libspotify/api.h>
}


XplodifySession::XplodifySession() 
    :m_session(NULL)
    ,m_plcontainer()
    ,m_playlist()
    ,m_track()
    ,m_handler(NULL)
    ,m_uuid("")
    ,m_loggedin(false)
    ,m_playback_done(1)
    ,m_remove_tracks(0)
    ,m_track_idx(-1)
{
    //EMPTY
}

XplodifySession::XplodifySession(XplodifyHandler * h) 
    :m_session(NULL)
    ,m_plcontainer()
    ,m_playlist()
    ,m_track()
    ,m_handler(h)
    ,m_uuid("")
    ,m_loggedin(false)
    ,m_playback_done(1)
    ,m_remove_tracks(0)
    ,m_track_idx(-1)
{
        //assign ssession later.

}

XplodifySession::~XplodifySession()
{
    //empty
}

boost::shared_ptr< XplodifySession > XplodifySession::create(XplodifyHandler * h)
{
    if(!h) {
            return boost::shared_ptr< XplodifySession >( new XplodifySession() );
    }

    return boost::shared_ptr< XplodifySession >( new XplodifySession(h) );
}

XplodifySession * XplodifySession::get_session_from_udata(sp_session * sp) {
    XplodifySession * s = 
        reinterpret_cast<XplodifySession *>(sp_session_userdata(sp));

    return s;
}


int XplodifySession::init_session(const uint8_t * appkey, size_t appkey_size) {

    sp_error err;

    m_spconfig.api_version = SPOTIFY_API_VERSION;
    m_spconfig.cache_location = "tmp";
    m_spconfig.settings_location = "tmp";
    m_spconfig.application_key = appkey;
    m_spconfig.application_key_size = appkey_size; // Set in main()
    m_spconfig.user_agent = "spotifyd";
    m_spconfig.userdata = this; //we'll use this in callbacks

    //get callbacks ready
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
#if 0 //PENDING
    pl_callbacks.tracks_added = &tracks_added;
    pl_callbacks.tracks_removed = &tracks_removed;
    pl_callbacks.tracks_moved = &tracks_moved;
    pl_callbacks.playlist_renamed = &playlist_renamed;
#endif
    m_spconfig.callbacks = &session_callbacks;

    err = sp_session_create( &m_spconfig, &m_session );
    if( err != SP_ERROR_OK) {
        return -1;
    }

    //check errors, blah....
    return 0;
}

void XplodifySession::login( const std::string& username
                          , const std::string& passwd
                          , bool remember ) {


    //gotta add blob support (see libspotify api).
    sp_error err;
    err = sp_session_login( 
            m_session, 
            username.c_str(), 
            passwd.c_str(), remember, NULL);

}

boost::shared_ptr<XplodifyPlaylistContainer> XplodifySession::get_pl_container(void) {
    if(!!m_plcontainer) {
        return m_plcontainer;
    }

    sp_playlistcontainer* c = sp_session_playlistcontainer( m_session );

    if (NULL == c)
    {
        return boost::shared_ptr<XplodifyPlaylistContainer>();
    }

    m_plcontainer.reset(new XplodifyPlaylistContainer(shared_from_this()));

    if (!m_plcontainer->load(c)) {
        m_plcontainer.reset();
    }

    return m_plcontainer;
}

void XplodifySession::set_active_playlist(int idx) {
    boost::shared_ptr<XplodifyPlaylistContainer> pc = get_pl_container();
    if(!pc) {
        return;
    }

    boost::shared_ptr<XplodifyPlaylist> pl = pc->get_playlist(idx);
    m_playlist = pl;
}

void XplodifySession::set_active_playlist(std::string plname) {
    boost::shared_ptr<XplodifyPlaylistContainer> pc = get_pl_container();
    if(!pc) {
        return;
    }

    boost::shared_ptr<XplodifyPlaylist> pl = pc->get_playlist(plname);
    m_playlist = pl;
}

std::string XplodifySession::get_playlist_name(void) {
    if(!m_playlist) {
        return std::string("");
    }

    return m_playlist->get_name();
}

sp_session * XplodifySession::get_sp_session() {
    return m_session;
}

void XplodifySession::set_track(int idx) {
    if( idx < 0 ) {
        m_track_idx = NO_TRACK_IDX;
        return;
    }

    if(!m_playlist) {
        return;
    }

    boost::shared_ptr<XplodifyTrack> track = m_playlist->get_track_at(idx);
    //track has been changed.
    if(m_track && track != m_track) {
        sp_session_player_unload(m_session);
        m_handler->audio_fifo_flush_now();
        m_track_idx = NO_TRACK_IDX;
        m_track = boost::shared_ptr<XplodifyTrack>();
    }
    if(!track || track->get_track_error() != SP_ERROR_OK ) {
        return;
    }
    if(m_track == track) {
        return;
    }
    m_track = track;
    m_track_idx = idx;
    sp_session_player_load(m_session, m_track->m_track);
}

#if 0
void selectPlaylist(const SpotifyCredential& cred, const std::string& playlist) {
    // Your implementation goes here
    printf("selectPlaylist\n");
}
#endif


void XplodifySession::end_of_track() {
#if 0
    lock();
    m_playback_done = 1;
    //h->setPlaybackState(DONE);
    switchSession();
    cond_signal();
    unlock();
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

void XplodifySession::logged_in(sp_session *sess, sp_error error) {
    //We've logged in succesfully, lets load pl container, and pl's
    m_plcontainer = get_pl_container(); 

    return;
}

void XplodifySession::flush() {
    boost::shared_ptr<XplodifyPlaylistContainer> pc = get_pl_container();
    if(!pc) {
        return;
    }

    pc->flush();
}

void XplodifySession::start_playback()
{
    sp_session_player_play(m_session, 1);
    return;
}

void XplodifySession::stop_playback()
{
    sp_session_player_play(m_session, 0);
    return;
}

void XplodifySession::get_audio_buffer_stats(sp_audio_buffer_stats * stats) {
    m_handler->audio_fifo_stats(stats);
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

    int n_frames;

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

    XplodifySession * s = XplodifySession::get_session_from_udata(sess);
    if(!s) {
        return 0;
    }

    s->music_delivery(sess, format, frames, num_frames);
    return 0; //or whatever...
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

    XplodifySession * s = XplodifySession::get_session_from_udata(sess);
    if(!s) {
        return;
    }

    s->end_of_track();
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
