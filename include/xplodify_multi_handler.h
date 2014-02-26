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
    private:
        XplodifyPlaybackManager      m_playmgr;
};

#endif //_XPLODIFY_HANDLER_H

