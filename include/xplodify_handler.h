#ifndef _XPLODIFY_HANDLER_H
#define _XPLODIFY_HANDLER_H

#include <cstdint>
#include <ctime>

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

#include "spotify_handler.h"
#include "spotify_data.h"


class XplodifyHandler
    : public SpotifyHandler
{
    public:
        XplodifyHandler();
        virtual ~XplodifyHandler();

        virtual bool handler_available();
        virtual std::string check_in();
        virtual bool check_out(const std::string& uuid);
        virtual bool login(const std::string& uuid, 
                const std::string& username, const std::string& passwd);
        virtual bool login(const std::string& uuid, const std::string& token);
        virtual bool login_status(std::string uuid);
        virtual bool logout(std::string uuid);
        virtual std::vector< 
            boost::shared_ptr<XplodifyPlaylist> > get_playlists(std::string uuid);
        virtual std::vector< 
            boost::shared_ptr<XplodifyTrack> > get_tracks(std::string uuid, int pid);
        virtual std::vector< 
            boost::shared_ptr<XplodifyTrack> > get_tracks(
                    std::string uuid, const std::string& name);
        virtual bool select_playlist(std::string uuid, int pid);
        virtual bool select_playlist(std::string uuid, std::string pname);
        virtual bool select_track(std::string uuid, int tid);
        virtual bool select_track(std::string uuid, std::string tname);
        virtual boost::shared_ptr<XplodifyTrack> whats_playing(std::string uuid);
        virtual void play();
        virtual void stop();

        virtual void notify_main_thread(void);
        virtual void set_session_done(bool done);
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

        sess_map                     m_session_cache;
        sess_map_sequenced::iterator m_sess_it;

        size_t get_cache_size() {
            sess_map_sequenced& smap = m_session_cache.get<0>();

            return smap.size();
        }

        void remove_from_cache(const std::string& uuid);
        bool exists_in_cache(const std::string& uuid);

        boost::shared_ptr<XplodifySession> get_session(const std::string& uuid);
        boost::shared_ptr<XplodifySession> get_session(const sp_session * sps);
};

#endif //_XPLODIFY_HANDLER
