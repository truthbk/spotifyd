#ifndef _SPOTIFY_CUST_HH
#define _SPOTIFY_CUST_HH

#include <cstdint>
#include <ctime>
#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "lockable.h"
#include "runnable.h"

extern "C" {
	#include <libspotify/api.h>
	#include "audio.h"
}

/* --- Data --- */
const uint8_t g_appkey[] = {
0x01, 0xD7, 0x2B, 0x8C, 0xC7, 0xB9, 0xBE, 0xBF, 0x88, 0x45, 0xBE, 0x56, 0xDE, 0xCA, 0x4A, 0x11,
	0xB9, 0x6B, 0xF9, 0x3F, 0xA2, 0x77, 0x9A, 0xD3, 0x68, 0xE9, 0x4F, 0x1C, 0xFF, 0x6D, 0x70, 0x41,
	0x2A, 0x35, 0x6E, 0x1D, 0x87, 0x37, 0xA3, 0x21, 0xFD, 0xFA, 0x37, 0x8D, 0xDA, 0xE4, 0xD9, 0x98,
	0x4D, 0x07, 0x01, 0x31, 0x6B, 0xEB, 0x35, 0xAC, 0xAA, 0x4A, 0xF2, 0x04, 0xB2, 0x8C, 0x81, 0x31,
	0xF9, 0x9E, 0x9C, 0x84, 0x1B, 0xB7, 0x48, 0x93, 0x34, 0x8A, 0x45, 0x7F, 0xAA, 0xBD, 0x04, 0x60,
	0x72, 0x78, 0x06, 0x3E, 0x97, 0xDA, 0x36, 0x5D, 0xBB, 0x0A, 0x37, 0x69, 0x27, 0x3B, 0x49, 0x9D,
	0xB1, 0x68, 0x14, 0x80, 0x13, 0x50, 0xD5, 0x8D, 0xAD, 0xF9, 0xDF, 0xC0, 0xDE, 0x82, 0x7D, 0x42,
	0x69, 0x9F, 0xE5, 0xCB, 0x7A, 0x05, 0xE3, 0x88, 0x27, 0x18, 0x2C, 0x05, 0x1B, 0x04, 0x51, 0xA4,
	0x9F, 0x66, 0xDF, 0x4B, 0x39, 0xF7, 0x56, 0x8E, 0xC4, 0xB1, 0x81, 0xDE, 0xF0, 0x11, 0x9C, 0x2C,
	0x22, 0x69, 0xC8, 0x0D, 0x6F, 0xAB, 0x8E, 0x6B, 0x60, 0x21, 0xF9, 0x64, 0xD2, 0x85, 0x7F, 0x65,
	0x6F, 0x97, 0x53, 0x62, 0x78, 0x7B, 0x79, 0x8A, 0x83, 0x3C, 0x66, 0xA2, 0x97, 0xC4, 0x0A, 0xAB,
	0x30, 0xA4, 0x37, 0x2D, 0xE0, 0xDF, 0xD8, 0x68, 0x91, 0x72, 0xCA, 0x36, 0xD5, 0xEB, 0xA1, 0x5A,
	0x76, 0xE8, 0xAF, 0x24, 0xFC, 0x64, 0x74, 0x69, 0x76, 0xE6, 0x30, 0xC8, 0xFE, 0xA1, 0x28, 0x25,
	0x90, 0x1F, 0x68, 0xBD, 0x2D, 0xF6, 0x75, 0x0A, 0x1D, 0xF5, 0x97, 0x24, 0x60, 0x39, 0x3C, 0xC0,
	0xC9, 0x52, 0x96, 0x0F, 0x9F, 0x0B, 0x20, 0x09, 0xD9, 0xDE, 0xE6, 0x8B, 0x69, 0x98, 0x99, 0x81,
	0xAD, 0x17, 0x23, 0x67, 0x30, 0x23, 0xD5, 0x9E, 0x50, 0x67, 0xB3, 0xB2, 0x9E, 0x66, 0xB2, 0x5F,
	0x21, 0x2B, 0xA9, 0x2B, 0xA2, 0x52, 0x9D, 0xCB, 0xF8, 0x1D, 0x94, 0x56, 0xAC, 0xA9, 0x63, 0xC8,
	0x5B, 0xF1, 0x74, 0xE1, 0xDB, 0x13, 0xF1, 0xB4, 0xC5, 0x79, 0x77, 0x94, 0x07, 0xF5, 0x1D, 0x08,
	0xC1, 0xE9, 0xBF, 0xF5, 0x72, 0x97, 0x4D, 0xE2, 0xFB, 0x3B, 0xE1, 0x1A, 0x5C, 0xF6, 0x73, 0xDA,
	0x53, 0x28, 0xA0, 0x29, 0xAF, 0xCC, 0x8E, 0x2D, 0xC0, 0x11, 0xD6, 0x4F, 0x7B, 0x9D, 0x14, 0x23,
	0x5B,};

const size_t g_appkey_size = sizeof(g_appkey);

#define SP_TMPDIR "/tmp/spotifyd"


//Forward declaration
class XplodifySession;


// This baby here, the XplodifyHandler, was originally conceived as a singleton. The 
// main idea behind that decision was the fact that we are only supposed to have a 
// single audio queue. Although perhaps fundamentally correct, because singleton's are
// very often a bad decision which may bring nast consequences eventually, I rather leave 
// that decision to the coder - if he decides to to implement several  XplodifyHandler, 
// instances.... then he will be responsible for his actions ;)
//
// The Handler manages incoming thrift requests and most of the logic.
// If we've got several ongoing spotify sessions for different users, the idea is to 
// randomly play music from each of their 'selected' playlists. 
//
// For example: if Joe is listening to his Hip Hop playist, and Jane is listening
// to her oldies playlists. We *do not* want several instances of the XplodifyHandler
// created. We don't need them either. The Handler will randomly select tracks from
// each of the users registered. The Audio Queue must be a singleton, and so we 
// can make the entire handler singleton. One instance is enough to handler all
// incoming requests.
//
class XplodifyHandler 
        : virtual public SpotifyIf
        , public Runnable
        , private Lockable {
    public:
        XplodifyHandler();
        void loginSession(SpotifyCredential& _return, const SpotifyCredential& cred);
        bool isLoggedIn(const SpotifyCredential& cred);
        int64_t getStateTS(const SpotifyCredential& cred);
        void logoutSession(const SpotifyCredential& cred);
        void sendCommand(const SpotifyCredential& cred, const SpotifyCmd::type cmd);
        void switch_session();

        void search(SpotifyPlaylist& _return, const SpotifyCredential& cred,
			const SpotifySearch& criteria);
        void getPlaylists(SpotifyPlaylistList& _return, const SpotifyCredential& cred);
        void getPlaylist(SpotifyPlaylist& _return, const SpotifyCredential& cred,
			const int32_t plist_id);
        void getPlaylistByName(SpotifyPlaylist& _return, const SpotifyCredential& cred,
			const std::string& name);
        void selectPlaylist(const SpotifyCredential& cred, const std::string& playlist);
        void selectPlaylistById(const SpotifyCredential& cred, const int32_t plist_id);
        void selectTrack(const SpotifyCredential& cred, const std::string& track);
        void selectTrackById(const SpotifyCredential& cred, const int32_t track_id);
        bool merge2playlist(const SpotifyCredential& cred, const std::string& pl,
			const SpotifyPlaylist& tracks);
        bool add2playlist(const SpotifyCredential& cred, const std::string& pl,
			const SpotifyTrack& track);
        void whats_playing(SpotifyTrack& _return);

        void notify_main_thread(void);
        void set_playback_done(bool done);
        int  music_playback(const sp_audioformat * format, 
                const void * frames, int num_frames);
        void audio_fifo_stats(sp_audio_buffer_stats *stats);
        void audio_fifo_flush_now();
        void update_timestamp();
        std::string get_tmpdir();


    protected:
        //implementing runnable
        void run();
    private:
        struct sess_map_entry {
            std::string _uuid;
            std::uintptr_t _sessintptr;
            int _ptrlow32;

            boost::shared_ptr<XplodifySession> session;

            sess_map_entry( const std::string &uuid, uintptr_t sintptr
                    , boost::shared_ptr<XplodifySession> sess ) 
                : _uuid(uuid)
                , _sessintptr(sintptr)
                  //lower 32 bits for greater entropy.
                , _ptrlow32( static_cast<int>(_sessintptr))
                , session(sess)
            {
                return;
            }
            sess_map_entry( const std::string &uuid, const sp_session * sp
                    , boost::shared_ptr<XplodifySession> sess ) 
                : _uuid(uuid)
                , _sessintptr(reinterpret_cast<std::uintptr_t>(sp))
                , session(sess)
            {
            }
        };

        //maybe tagging would make code more readable, but I'm not a fan. Leaving out for now.
        typedef boost::multi_index_container<
            sess_map_entry,
            boost::multi_index::indexed_by<
                boost::multi_index::sequenced<>,
                boost::multi_index::hashed_unique< 
                    BOOST_MULTI_INDEX_MEMBER(sess_map_entry, std::string, _uuid) >,
                boost::multi_index::hashed_unique< 
                    BOOST_MULTI_INDEX_MEMBER(sess_map_entry, std::uintptr_t, _sessintptr) >
            > > sess_map;

        typedef sess_map::nth_index<0>::type sess_map_sequenced;
        typedef sess_map::nth_index<1>::type sess_map_by_uuid;
        typedef sess_map::nth_index<2>::type sess_map_by_sessptr;
        sess_map m_session_cache;

        void remove_from_cache(const std::string& uuid) {
            sess_map_entry aux_entry(*m_sess_it);
            sess_map_by_uuid& sess_by_uuid = m_session_cache.get<1>();
            sess_by_uuid.erase(uuid);

            //fix the potentially invalidated iterator.
            m_sess_it = m_session_cache.get<0>().iterator_to(aux_entry);
        }

        //no transfer of ownership, we're good with raw pointers.
        typedef std::map<std::string, boost::asio::deadline_timer *> timer_map;
        timer_map m_timers;
        boost::asio::io_service m_io;
        const size_t LOGIN_TO = 1;


        void login_timeout(const boost::system::error_code&,
                std::string uuid);
        boost::shared_ptr<XplodifySession> get_session(const std::string& uuid);
        boost::shared_ptr<XplodifySession> get_session(const sp_session * sps);
        boost::shared_ptr<XplodifySession> getActiveSession(void);
        void setActiveSession(boost::shared_ptr<XplodifySession> session);


        //we also need to be able to search by sp_session, that's quite important; 
        //callbacks rely very heavily on it.

        sp_playlistcontainer *                  getPlaylistContainer(SpotifyCredential& cred);
        audio_fifo_t *                          audio_fifo();

        //libspotify wrapped
        audio_fifo_t                            m_audiofifo;

        //proper members
        sess_map_sequenced::iterator            m_sess_it;

        boost::shared_ptr<XplodifySession>      m_active_session;

        int                                     m_playback_done;
        int                                     m_notify_events;
        std::string                             m_sp_tmpdir;
        std::time_t                             m_ts;

        //SILENCE NUM SAMPLES THRESHOLD
        enum { SILENCE_N_SAMPLES = 8192 };

};
#endif
