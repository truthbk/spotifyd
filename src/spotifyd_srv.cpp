// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include <protocol/TBinaryProtocol.h>
#include <server/TSimpleServer.h>
#include <transport/TServerSocket.h>
#include <transport/TBufferTransports.h>

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

#if 0
#include <libspotify/api.h>
#include "audio.h"
#endif

//I don't believe these headers are C++ ready....
extern "C" {
#include <libspotify/api.h>
#include "audio.h"
}

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

//define the operators
//these must be moved to their own file with other possible
//definitions for thrift generated code.
bool SpotifyTrack::operator < (const SpotifyTrack & other) const 
{
    return (this->_id - other._id) < 0;
}

bool SpotifyCredential::operator < (const SpotifyCredential & other) const 
{
    return this->_username.compare(other._username) < 0;
}


SpotifySession::SpotifySession() 
    :m_sess(NULL)
    ,m_jukeboxlist(NULL)
    ,m_currenttrack(NULL)
    ,m_handler(NULL)
    ,m_notify_do(0)
    ,m_playback_done(1)
    ,m_remove_tracks(0)
    ,m_track_idx(-1)
    ,m_uuid("")
    ,m_loggedin(false)
{
        //assign ssession later.

}

SpotifySession::~SpotifySession()
{
    //empty
}

boost::shared_ptr< SpotifySession > SpotifySession::create()
{
    return boost::shared_ptr< SpotifySession >( new SpotifySession() );
}

SpotifySession * SpotifySession::getSessionFromUData(sp_session * sp) {
    SpotifySession * s = 
        reinterpret_cast<SpotifySession *>(sp_session_userdata(sp));

    return s;
}


int SpotifySession::initSession(SpotifyHandler * const h, 
        const uint8_t * appkey, size_t appkey_size) {

    sp_error err;

    m_handler = h;

    m_spconfig.api_version = SPOTIFY_API_VERSION;
    m_spconfig.cache_location = "tmp";
    m_spconfig.settings_location = "tmp";
    m_spconfig.application_key = appkey;
    m_spconfig.application_key_size = appkey_size; // Set in main()
    m_spconfig.user_agent = "spotifyd";
    m_spconfig.callbacks = &session_callbacks;
    m_spconfig.callbacks = &session_callbacks;
    m_spconfig.userdata = this; //we'll use this in callbacks

    err = sp_session_create( &m_spconfig, &m_sess );
    if( err != SP_ERROR_OK) {
        return -1;
    }

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
    //check errors, blah....
    return 0;
}

void SpotifySession::login( const std::string& username
                          , const std::string& passwd
                          , bool remember ) {


    //gotta add blob support (see libspotify api).
    sp_session_login( 
            m_sess, 
            username.c_str(), 
            passwd.c_str(), remember, NULL);

}

sp_playlistcontainer * SpotifySession::getPlaylistContainer(void) {
    if(!m_sess) {
        return NULL;
    }
    return sp_session_playlistcontainer(m_sess);
}
void SpotifySession::setActivePlaylist(sp_playlist * pl) {
    if(pl) {
        m_jukeboxlist = pl;
    }
}
std::string SpotifySession::getPlaylistName(void) {
    if(m_jukeboxlist) {
        return std::string("");
    }

    return std::string(sp_playlist_name(m_jukeboxlist));
}

sp_track * SpotifySession::setCurrentTrack(int idx) {
#define NOTRACK -1
    if( idx < 0 ) {
        m_track_idx = NO_TRACK_IDX;
        m_currenttrack = NO_TRACK;
        return NO_TRACK;
    }

    sp_track * t = NULL;
    int n_tracks = 0;

    if(!m_jukeboxlist) {
        return NO_TRACK;
    }

    n_tracks = sp_playlist_num_tracks(m_jukeboxlist);
    if(!n_tracks || n_tracks < idx) {
        return NO_TRACK;
    }

    t = sp_playlist_track(m_jukeboxlist, idx);
    m_currenttrack = t;
    m_track_idx = idx;

    return t;

}

#if 0
void selectPlaylist(const SpotifyCredential& cred, const std::string& playlist) {
    // Your implementation goes here
    printf("selectPlaylist\n");
}
#endif


SpotifyHandler::SpotifyHandler()
    : Runnable()
    , Lockable()
    , m_sess_it(m_session_cache.get<0>().begin())
    , m_playback_done(1)
    , m_notify_events(0)
{
    //Nothing else
}


void SpotifyHandler::run() 
{
    int next_timeout = 0;

    while(!m_done)
    {
        if (next_timeout == 0) {
            while(!m_notify_events && !m_playback_done)
                cond_wait();
        } else {
            struct timespec ts;

#if _POSIX_TIMERS > 0
            clock_gettime(CLOCK_REALTIME, &ts);
#else
            struct timeval tv;
            gettimeofday(&tv, NULL);
            TIMEVAL_TO_TIMESPEC(&tv, &ts);
#endif
            ts.tv_sec += next_timeout / 1000;
            ts.tv_nsec += (next_timeout % 1000) * 1000000;
            
            cond_timedwait(&ts);
        }

        m_notify_events = 0;
        unlock();

        if (m_playback_done) {
            //track_ended();
            m_playback_done = 0;
        }

        do {
            std::set<sp_session *>::iterator sess_it;
            for( sess_it = m_event_spsessions.begin() ;
                    sess_it != m_event_spsessions.end() ;
                    sess_it++ )
            sp_session_process_events(*sess_it, &next_timeout);
        } while (next_timeout == 0);

        lock();
    }
}

//Should we include this in an anonymous namespace once we put relevant stuff 
//in its namespace?
static SpotifyHandler* getHandlerFromUserData( sp_session* session )
{
    SpotifyHandler* pHandler = 
        reinterpret_cast<SpotifyHandler*>(sp_session_userdata(session));
    return pHandler;
}


void SpotifyHandler::loginSession(SpotifyCredential& _return, const SpotifyCredential& cred) {
    // Your implementation goes here
    sp_error err;
#ifdef DEBUG
    printf("initiatingSession\n");
#endif

    boost::shared_ptr< SpotifySession > sess = getSession(cred._uuid);
    if(!sess) {
        sess = SpotifySession::create();
        sess->initSession( this, g_appkey, g_appkey_size );

        sess->login(cred._username, cred._passwd);


        lock();
        //generate UUID
        const boost::uuids::uuid uuid = boost::uuids::random_generator()();
        const std::string uuid_str = boost::lexical_cast<std::string>(uuid);

        m_session_cache.get<1>().insert(sess_map_entry( uuid_str, 
                    const_cast<const sp_session *>(sess->getSession()), sess ));

        _return = cred;
        _return.__set__uuid(uuid_str);
        unlock();
    }
}

bool SpotifyHandler::isLoggedIn(const SpotifyCredential& cred) {

    boost::shared_ptr< SpotifySession > 
        sess = getSession(cred._uuid);

    if(!sess)
        return false;

    return sess->getLoggedIn();
}


void SpotifyHandler::logoutSession(const SpotifyCredential& cred) {

    sp_error err;
    bool fix_iter = false;

    boost::shared_ptr<SpotifySession> sess = getSession(cred._uuid);
    if(!sess) {
        return;
    }

    lock();
    if(sess == m_active_session) 
    {
        //stop track move onto next session.
        switchSession();
    }
    err = sp_session_logout(sess->getSession());

    //iterators are not invalidated by insertion/removal in maps!
    if(err == SP_ERROR_OK )
    {
        sess_map_by_uuid& sessByUuid = m_session_cache.get<1>();
        sessByUuid.erase(cred._uuid);

        //fix the invalidated iterator.
        m_sess_it = m_session_cache.get<0>().begin();
        while (m_sess_it->session != m_active_session) {
            m_sess_it++;
        }
    }
    unlock();

    return;
}

void SpotifyHandler::sendCommand(const SpotifyCredential& cred, const SpotifyCmd::type cmd) {
    // Your implementation goes here
    printf("sendCommand\n");

    boost::shared_ptr<SpotifySession> sess = getSession(cred._uuid);
    if(!sess) {
        return;
    }

    switch(cmd){
	case SpotifyCmd::PLAY:
	    break;
	case SpotifyCmd::PAUSE:
	    break;
	case SpotifyCmd::NEXT:
	    break;
	case SpotifyCmd::PREV:
	    break;
	case SpotifyCmd::RAND:
	    break;
            case SpotifyCmd::LINEAR:
	    break;
	case SpotifyCmd::REPEAT_ONE:
	    break;
	case SpotifyCmd::REPEAT:
	    break;
	default:
	    break;
    }
    return;
}

//change session, allow it to be played.
void SpotifyHandler::switchSession() {
    sp_session_player_unload(m_active_session->getSession());

    //Currently just round-robin.
    if (++m_sess_it == m_session_cache.get<0>().end())
    {
        m_sess_it = m_session_cache.get<0>().begin();
    }

    m_active_session = m_sess_it->session;
    //should load track...
    //sp_session_player_load(m_active_session->getSession(), sometrack );

    return;
}

void SpotifyHandler::tracks_added(sp_playlist *pl, sp_track * const *tracks,
	int num_tracks, int position, void *userdata) {
    boost::shared_ptr<SpotifySession> sess = getActiveSession();
    if(!sess) {
	return;
    }

    if (pl != sess->getActivePlaylist())
    {
	return;
    }

    printf("jukebox: %d tracks were added\n", num_tracks);
    fflush(stdout);
    //try_jukebox_start();
}

void SpotifyHandler::tracks_removed(sp_playlist *pl, const int *tracks,
	int num_tracks, void *userdata) {
    int i, k = 0;

    boost::shared_ptr<SpotifySession> sess = getActiveSession();
    if(!sess) {
	return;
    }

    if (pl != sess->getActivePlaylist())
    {
	return;
    }

    int track_idx = sess->getCurrentTrackIdx();
    for (i = 0; i < num_tracks; ++i)
    {
	if (tracks[i] < track_idx)
	{
	    ++k;
	}
    }

    track_idx -= k;
    sess->setCurrentTrack(track_idx);

    fflush(stdout);
    //try_jukebox_start();
}

void SpotifyHandler::tracks_moved(sp_playlist *pl, const int *tracks,
	int num_tracks, int new_position, void *userdata) {
    boost::shared_ptr<SpotifySession> sess = getActiveSession();
    if(!sess) {
	return;
    }

    if (pl != sess->getActivePlaylist())
    {
	return;
    }

    printf("jukebox: %d tracks were moved around\n", num_tracks);
    fflush(stdout);

    //try_jukebox_start();
}

void SpotifyHandler::playlist_renamed(sp_playlist *pl, void *userdata) {
    const char *name = sp_playlist_name(pl);
    const char *currentpl = NULL;

    boost::shared_ptr<SpotifySession> sess = getActiveSession();
    if(!sess) {
	return;
    }
    currentpl = sp_playlist_name(sess->getActivePlaylist());

    if (!strcasecmp(name, currentpl)) 
    {
	sess->setActivePlaylist(pl);
	sess->setCurrentTrack(0);
	//try_jukebox_start();

    } else if (sess->getActivePlaylist() == pl) {
	printf("jukebox: current playlist renamed to \"%s\".\n", name);
	sess->setActivePlaylist(NULL);
	sess->setCurrentTrack(NOTRACK);
	//sp_session_player_unload(g_sess);
    }
}

void SpotifySession::logged_in(sp_session *sess, sp_error error) {
    //get the session from the session list...

    //TODO: check out what we got in sp_error error.
    if(error != SP_ERROR_OK) {
        sp_error_message(error);
        return;
    }

    m_loggedin = true;

    //What else??


#if 0
    //Need to store those in the handler, fo sho.
    sp_playlistcontainer *pc = sp_session_playlistcontainer(sess);
    int i;

    for (i = 0; i < sp_playlistcontainer_num_playlists(pc); ++i)
    {
	//we add the playlists to the list. Select the user's playlist as well.
	sp_playlist *pl = sp_playlistcontainer_playlist(pc, i);
	sp_playlist_add_callbacks(pl, &pl_callbacks, NULL);

    }
#endif

}

void SpotifySession::end_of_track(sp_session *sess) {
#if 0
    lock();
    m_playback_done = 1;
    //h->setPlaybackState(DONE);
    switchSession();
    cond_signal();
    unlock();
#endif
}

void SpotifySession::play_token_lost(sp_session *sess)
{
#if 0
    audio_fifo_flush(&m_audiofifo);

    //Find the session and stop it.
    boost::shared_ptr<SpotifySession> spsession = getActiveSession();

    if (spsession->getCurrentTrack() != NULL)
    {
	// unload the session that caused the token loss.
	sp_session_player_unload(sess);
	spsession->setCurrentTrack(NO_TRACK_IDX);
    }
    switchSession();
#endif
}

int SpotifySession::music_delivery(sp_session *sess, const sp_audioformat *format,
	const void *frames, int num_frames)
{
#if 0
    size_t s;
    audio_fifo_data_t *afd;

    if (num_frames == 0)
    {
	return 0; // Audio discontinuity, do nothing
    }

    pthread_mutex_lock(&m_audiofifo.mutex);

    /* Buffer one second of audio */
    if (m_audiofifo.qlen > format->sample_rate)
    {
	pthread_mutex_unlock(&m_audiofifo.mutex);
	return 0;
    }

    s = num_frames * sizeof(int16_t) * format->channels;

    //dont want to malloc, change this to new....
    afd = (audio_fifo_data_t *) malloc(sizeof(audio_fifo_data_t) + s);
    memcpy(afd->samples, frames, s);

    afd->nsamples = num_frames;
    afd->rate = format->sample_rate;
    afd->channels = format->channels;

    TAILQ_INSERT_TAIL(&m_audiofifo.q, afd, link);
    m_audiofifo.qlen += num_frames;

    pthread_cond_signal(&m_audiofifo.cond);
    pthread_mutex_unlock(&m_audiofifo.mutex);

    return num_frames;
#endif
}

void SpotifySession::notify_main_thread(sp_session *sess)
{
#if 0
    lock();

    m_event_spsessions.insert(sess);
    m_notify_events = 1;

    unlock();
#endif
}


void SpotifyHandler::search(SpotifyPlaylist& _return, const SpotifyCredential& cred,
		const SpotifySearch& criteria) {
    // Your implementation goes here
    printf("search\n");
    boost::shared_ptr<SpotifySession> sess = getSession(cred._uuid);
    if(!sess) {
        return;
    }
}

void SpotifyHandler::getPlaylists(SpotifyPlaylistList& _return, const SpotifyCredential& cred) 
{
    printf("getPlaylists\n");
    boost::shared_ptr<SpotifySession> sess = getSession(cred._uuid);
    if(!sess) {
        return;
    }
    sp_playlistcontainer * pc = sess->getPlaylistContainer();
    if(!pc) {
	return;
    }

    for (int i = 0; i < sp_playlistcontainer_num_playlists(pc); ++i)
    {
	sp_playlist *pl = sp_playlistcontainer_playlist(pc, i);

	std::string s_pl(sp_playlist_name(pl));
	_return.insert(s_pl);
    }

    return;

}

void SpotifyHandler::getPlaylist(SpotifyPlaylist& _return, const SpotifyCredential& cred,
		const int32_t plist_id) {
    // Your implementation goes here
    printf("getPlaylist\n");

    boost::shared_ptr<SpotifySession> sess = getSession(cred._uuid);
    if(!sess) {
	return;
    }

    sp_playlistcontainer * pc = sess->getPlaylistContainer();
    if(!pc) {
	return;
    }

    sp_playlist *pl = sp_playlistcontainer_playlist(pc, plist_id);

    if(!pl) {
	return;
    }

    for(int j = 0 ; j < sp_playlist_num_tracks(pl) ; j++ ) {
	sp_track * t = sp_playlist_track(pl, j);
	int duration = sp_track_duration(t); //millisecs?
	boost::shared_ptr<SpotifyTrack> spt(new SpotifyTrack());

	spt->__set__name( std::string(sp_track_name(t)) );
        //I guess we'll have to support multiple artists in the future.
	spt->__set__artist( std::string( sp_artist_name(sp_track_artist(t, 0))));
	spt->__set__minutes( duration / 60000 );
	spt->__set__seconds( (duration / 1000) % 60 );
	spt->__set__popularity( sp_track_popularity(t) );
	spt->__set__starred( sp_track_is_starred(sess->getSession(), t) );

	_return.insert(*spt);

    }

    return;
}

void SpotifyHandler::getPlaylistByName(SpotifyPlaylist& _return, const SpotifyCredential& cred,
		const std::string& name) {
    // Your implementation goes here
    printf("getPlaylistByName\n");

    boost::shared_ptr<SpotifySession> sess = getSession(cred._uuid);
    if(!sess) {
	return;
    }


    sp_playlistcontainer * pc = sess->getPlaylistContainer();
    if(!pc) {
	return;
    }

    for (int i = 0; i < sp_playlistcontainer_num_playlists(pc); ++i) {
	sp_playlist *pl = sp_playlistcontainer_playlist(pc, i);

	if(!pl) {
	    continue;
	}

	std::string plname(sp_playlist_name(pl));
	if(boost::iequals(plname, name)) {
	    getPlaylist(_return, cred, i);
	    break;
	}

    }
    return;
}

void SpotifyHandler::selectPlaylist(const SpotifyCredential& cred, const std::string& playlist) {
    // Your implementation goes here
    printf("selectPlaylist\n");
    boost::shared_ptr<SpotifySession> sess = getSession(cred._uuid);
    if(!sess) {
	return;
    }


    sp_playlistcontainer * pc = sess->getPlaylistContainer();
    if(!pc) {
	return;
    }

    //O(n) ugly, but this is really the libspotify way of doing it :(
    for (int i = 0; i < sp_playlistcontainer_num_playlists(pc); ++i) {
	sp_playlist *pl = sp_playlistcontainer_playlist(pc, i);

	if(!pl) {
	    continue;
	}

	std::string plname(sp_playlist_name(pl));
	if(boost::iequals(plname, playlist)) {
	    sess->setActivePlaylist(pl);
	    break;
	}
    }
}
void SpotifyHandler::selectPlaylistById(const SpotifyCredential& cred, const int32_t plist_id) {
    // Your implementation goes here
    printf("selectPlaylist\n");
    boost::shared_ptr< SpotifySession > sess = getSession(cred._uuid);
    if(!sess) {
	return;
    }


    sp_playlistcontainer * pc = sess->getPlaylistContainer();
    if(!pc) {
	return;
    }

    sp_playlist *pl = sp_playlistcontainer_playlist(pc, plist_id);

    if(!pl) {
	return;
    }

    sess->setActivePlaylist(pl);
}

bool SpotifyHandler::merge2playlist(const SpotifyCredential& cred, const std::string& pl,
		const SpotifyPlaylist& tracks) {
    // Your implementation goes here
    printf("merge2playlist\n");
    return true;
}

bool SpotifyHandler::add2playlist(const SpotifyCredential& cred, const std::string& pl,
		const SpotifyTrack& track) {
    // Your implementation goes here
    printf("add2playlist\n");
    return true;
}

void SpotifyHandler::whats_playing(SpotifyTrack& _return) {
    // Your implementation goes here
    printf("whats_playing\n");
}

boost::shared_ptr<SpotifySession> SpotifyHandler::getSession(const std::string& uuid) {

    sess_map_by_uuid& sessByUuid = m_session_cache.get<1>();

    sess_map_by_uuid::iterator sit = sessByUuid.find(uuid);
    if( sit == m_session_cache.get<1>().end() ) {
        return boost::shared_ptr<SpotifySession>();
    }

    return sit->session;
}

boost::shared_ptr<SpotifySession> SpotifyHandler::getSession(const sp_session * sps) {

    sess_map_by_sessptr& sessByPtr = m_session_cache.get<2>();

    sess_map_by_sessptr::iterator sit = sessByPtr.find(reinterpret_cast<uintptr_t>(sps));
    if( sit == m_session_cache.get<2>().end() ) {
        return boost::shared_ptr<SpotifySession>();
    }

    return sit->session;
}

boost::shared_ptr<SpotifySession> SpotifyHandler::getActiveSession(void) {
    return m_active_session;
}
void SpotifyHandler::setActiveSession(boost::shared_ptr<SpotifySession> session) {
    m_active_session = session;
}

//we also need to be able to search by sp_session, that's quite important; callbacks rely very heavily
//on it.
sp_playlistcontainer * SpotifyHandler::getPlaylistContainer(SpotifyCredential& cred) {
    boost::shared_ptr<SpotifySession> sess = getSession(cred._uuid);
    if(!sess) {
	return NULL;
    }

    return sp_session_playlistcontainer(sess->getSession());
}

#if 0
SpotifyHandler::session_map& SpotifyHandler::get_sessions() {
    return m_sessions;
}
#endif

audio_fifo_t * SpotifyHandler::audio_fifo() {
    return &m_audiofifo;
}




/* --------------------------  PLAYLIST CALLBACKS  ------------------------- */
#if 0 //PENDING
void SP_CALLCONV SpotifySession::cb_tracks_added(
        sp_playlist *pl, sp_track * const *tracks,
        int num_tracks, int position, void *userdata)
{
    if(!g_handler) {
        return;
    }

    return g_handler->tracks_added(pl, tracks, num_tracks, position, userdata);
}

void SP_CALLCONV SpotifySession::cb_tracks_removed(
        sp_playlist *pl, const int *tracks,
        int num_tracks, void *userdata)
{
    if(!g_handler) {
        return;
    }

    return g_handler->tracks_removed(pl, tracks, num_tracks, userdata);
}

void SP_CALLCONV SpotifySession::cb_tracks_moved(
        sp_playlist *pl, const int *tracks,
        int num_tracks, int new_position, void *userdata)
{
    if(!g_handler) {
        return;
    }

    return g_handler->tracks_moved(pl, tracks, num_tracks, new_position, userdata);
}

void SP_CALLCONV SpotifySession::cb_playlist_renamed(
        sp_playlist *pl, void *userdata)
{
    if(!g_handler) {
        return;
    }

    return g_handler->playlist_renamed(pl, userdata);
}
#endif


/* --------------------------  SESSION CALLBACKS  ------------------------- */
void SP_CALLCONV SpotifySession::cb_logged_in(
        sp_session *sess, sp_error error) {

    SpotifySession * s = SpotifySession::getSessionFromUData(sess);

    s->logged_in(sess, error);
    return;
}

void SP_CALLCONV SpotifySession::cb_logged_out(sp_session *sess) {

    //TODO
}

void SP_CALLCONV SpotifySession::cb_metadata_updated(sp_session *sess) {

    //TODO
    SpotifySession * s = SpotifySession::getSessionFromUData(sess);
}

void SP_CALLCONV SpotifySession::cb_connection_error(
        sp_session *sess, sp_error error) {

    //TODO
    SpotifySession * s = SpotifySession::getSessionFromUData(sess);
}

void SP_CALLCONV SpotifySession::cb_streaming_error(
        sp_session *sess, sp_error error) {

    //TODO
    SpotifySession * s = SpotifySession::getSessionFromUData(sess);
}

void SP_CALLCONV SpotifySession::cb_msg_to_user(
        sp_session *sess, const char * message) {

    //TODO
    SpotifySession * s = SpotifySession::getSessionFromUData(sess);
}

void SP_CALLCONV SpotifySession::cb_log_msg(
        sp_session *sess, const char * data) {

    //TODO
    SpotifySession * s = SpotifySession::getSessionFromUData(sess);
}

void SP_CALLCONV SpotifySession::cb_notify_main_thread(sp_session *sess)
{

    SpotifySession * s = SpotifySession::getSessionFromUData(sess);

    s->notify_main_thread(sess);
    return;
}

int SP_CALLCONV SpotifySession::cb_music_delivery(
        sp_session *sess, const sp_audioformat *format,
        const void *frames, int num_frames)
{

    SpotifySession * s = SpotifySession::getSessionFromUData(sess);

    s->music_delivery(sess, format, frames, num_frames);
    return 0; //or whatever...
}

void SP_CALLCONV SpotifySession::cb_play_token_lost(sp_session *sess) {

    SpotifySession * s = SpotifySession::getSessionFromUData(sess);

    s->play_token_lost(sess);
    return;
}

void SP_CALLCONV SpotifySession::cb_end_of_track(sp_session * sess) {

    SpotifySession * s = SpotifySession::getSessionFromUData(sess);

    s->end_of_track(sess);
    return;

}

void SP_CALLCONV SpotifySession::cb_userinfo_updated(sp_session * sp) {
    //TODO
    SpotifySession * s = SpotifySession::getSessionFromUData(sp);
    return;
}
void SP_CALLCONV SpotifySession::cb_start_playback(sp_session * sp) {
    //TODO
    SpotifySession * s = SpotifySession::getSessionFromUData(sp);
    return;
}
void SP_CALLCONV SpotifySession::cb_stop_playback(sp_session * sp) {
    //TODO
    SpotifySession * s = SpotifySession::getSessionFromUData(sp);
    return;
}
void SP_CALLCONV SpotifySession::cb_get_audio_buffer_stats(sp_session * sp,
        sp_audio_buffer_stats *stats) {
    //TODO
    SpotifySession * s = SpotifySession::getSessionFromUData(sp);
    return;
}

int main(int argc, char **argv) {
    int port = 9090;


    //SpotifyHandler
    boost::shared_ptr<SpotifyHandler> sHandler(new SpotifyHandler());

    //THRIFT Server
    boost::shared_ptr<TProcessor> processor(new SpotifyProcessor(sHandler));
    boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
    boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

    TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);

    server.serve();
    sHandler->start();

    return 0;
}

