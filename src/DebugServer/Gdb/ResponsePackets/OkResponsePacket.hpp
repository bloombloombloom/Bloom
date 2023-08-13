#pragma once

#include "ResponsePacket.hpp"

namespace DebugServer::Gdb::ResponsePackets
{
    /**
     * OK response packet expected by the GDB client, in response to certain commands.
     */
    class OkResponsePacket: public ResponsePacket
    {
    public:
        OkResponsePacket()
            : ResponsePacket(std::vector<unsigned char>{'O', 'K'})
        {}
    };
}
