#pragma once

#include "ResponsePacket.hpp"

namespace DebugServer::Gdb::ResponsePackets
{
    /**
     * Empty response packet expected by the GDB client, in response to certain commands.
     */
    class EmptyResponsePacket: public ResponsePacket
    {
    public:
        EmptyResponsePacket()
            : ResponsePacket(std::vector<unsigned char>{0})
        {}
    };
}
