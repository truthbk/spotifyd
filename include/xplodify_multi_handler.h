#ifndef _XPLODIFY_MULTI_HANDLER_H
#define _XPLODIFY_MULTI_HANDLER_H

#include "xplodify_handler.h"
#include "xplodify_playmgr.h"

class XplodifyMultiHandler
    : public XplodifyHandler
{
    public:
        XplodifyMultiHandler();
        XplodifyMultiHandler(std::string host,
                uint32_t port,
                uint8_t nprocs,
                std::string cachedir);

        void register_playback(std::string uuid);

        virtual bool select_playlist(std::string uuid, int pid);
        virtual bool select_playlist(std::string uuid, std::string pname);
        virtual bool select_track(std::string uuid, int tid);
        virtual bool select_track(std::string uuid, std::string tname);
        virtual boost::shared_ptr<XplodifyTrack> whats_playing(void);
        virtual boost::shared_ptr<XplodifyTrack> whats_playing(std::string uuid);
        virtual void play();
        virtual void stop();
        virtual void next();
        virtual void prev();
        virtual void set_playback_mode(SpotifyCmd::type cmd);
        virtual void set_repeat_mode(SpotifyCmd::type cmd);
    private:
        XplodifyPlaybackManager      m_playmgr;
};

#endif //_XPLODIFY_HANDLER_H

