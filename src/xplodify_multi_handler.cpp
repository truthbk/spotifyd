#include <string>

#include "xplodify_multi_handler.h"
#include "spotify_data.h"

using namespace xplodify::constants;

XplodifyMultiHandler::XplodifyMultiHandler()
    : XplodifyHandler()
    , m_playmgr(
            std::string("localhost"),
            IPCSRV_BASE_PORT,
            IPC_DEF_NPROCS )
{
}

XplodifyMultiHandler::XplodifyMultiHandler(
        std::string host,
        uint32_t port,
        uint8_t nprocs,
        std::string cachedir)
    : XplodifyHandler(cachedir)
    , m_playmgr(host, port, nprocs)
{
}
