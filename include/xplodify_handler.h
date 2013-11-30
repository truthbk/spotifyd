#ifndef _SPOTIFY_HANDLER_H
#define _SPOTIFY_HANDLER_H

#include <cstdint>
#include <ctime>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "spotify_data.h"

extern "C" {
    #include <libspotify/api.h>
    #include "audio.h"
}

class XplodifyHandler
    : public SpotifyHandler
{
    public:
        XplodifyHandler()
            : SpotifyHandler() {
        }
        virtual ~XplodifyHandler() {
        }

        virtual void login(const std::string& username, const::string& passwd);
        virtual void login(const std::string& token);
        virtual bool login_status(std::string uuid);
        virtual std::string logout(std::string uuid);
        virtual std::vector< std::string > get_playlists(string uuid);
        virtual std::vector< std::string > get_tracks(string uuid, int pid);
        virtual bool select_playlist(std::string uuid, int pid);
        virtual bool select_playlist(std::string uuid, std::string pname);
        virtual bool select_track(std::string uuid, int tid);
        virtual bool select_track(std::string uuid, std::string tname);
        virtual void play();
        virtual void stop();

        virtual void notify_main_thread(void);
        virtual void set_playback_done(bool done);
        virtual int  music_playback(const sp_audioformat * format, 
                const void * frames, int num_frames);
        virtual void audio_fifo_stats(sp_audio_buffer_stats *stats);
        virtual void audio_fifo_flush_now(void) ;

        virtual int64_t get_session_state(std::string uuid);
        virtual void update_timestamp(void);
        virtual std::string get_cachedir();

    protected:
        //implementing runnable
        virtual void run();
        virtual void switch_session();

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

        size_t get_cache_size() {
            sess_map_sequenced& smap = m_session_cache.get<0>();

            return smap.size();
        }

        void remove_from_cache(const std::string& uuid);

        boost::shared_ptr<XplodifySession> get_session(const std::string& uuid);
        boost::shared_ptr<XplodifySession> get_session(const sp_session * sps);
        boost::shared_ptr<XplodifySession> getActiveSession(void);
};

#endif //_XPLODIFY_HANDLER
