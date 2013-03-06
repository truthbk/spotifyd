#ifndef _SPOTIFY_TRACK_HH
#define _SPOTIFY_TRACK_HH

#include <cstdint>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "xplodify_sess.h"

extern "C" {
	#include <libspotify/api.h>
}


class XplodifyTrack
{
    public:
        XplodifyTrack(boost::shared_ptr<XplodifySession> sess);
        ~XplodifyTrack();

        bool          load(sp_track * track);
        void          unload();
        bool          is_loading();
        std::string   get_name();
        int           get_duration();
        int           get_num_artists();
        std::string   get_artist(int idx);
        bool          is_starred();
        int           get_disc();
        int           get_popularity();
        void          set_starred(bool star);
    private:
        sp_track *                            m_track;
        boost::shared_ptr<XplodifySession>    m_sess;
};

#endif
