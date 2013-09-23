#ifndef _SPOTIFY_TRACK_HH
#define _SPOTIFY_TRACK_HH

#include <cstdint>
#include <string>
#include <vector>

#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "xplodify_sess.h"
#include "cacheable.h"

extern "C" {
	#include <libspotify/api.h>
}


class XplodifyTrack
    : public Cacheable
{
    public:
        XplodifyTrack(boost::shared_ptr<XplodifySession> sess);
        ~XplodifyTrack();

        bool          load(sp_track * track);
        void          unload();
        bool          is_loaded();
        bool          is_streamable();
        std::string   get_name();
        int           get_index();
        int           get_duration();
        int           get_num_artists();
        std::string   get_artist(int idx);
        bool          is_starred();
        int           get_disc();
        int           get_popularity();
        void          set_starred(bool star);
        sp_error      get_track_error();

        //Cacheable purely virtual methods
        void cache(void);
        void uncache(void);
        bool is_cached(void);
            private:
                friend class XplodifySession;
                boost::weak_ptr<XplodifySession>      m_sess;
                sp_track *                            m_track;
};

#endif
