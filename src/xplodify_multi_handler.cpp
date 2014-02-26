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


void XplodifyMultiHandler::register_playback(std::string uuid) {
    user_map_by_uuid& user_by_uuid = m_user_cache.get<1>();

    user_map_by_uuid::iterator sit = user_by_uuid.find(uuid);
    if( sit == m_user_cache.get<1>().end() ) {
        return;
    }

    m_playmgr.register_user(sit->_user,sit->_passwd);
    return;
}
