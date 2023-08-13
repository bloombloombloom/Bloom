#pragma once

#include "ResponsePacket.hpp"

namespace DebugServer::Gdb::ResponsePackets
{
    /**
     * Error response packet expected by the GDB client, to indicate an error, in response to certain commands.
     */
    class ErrorResponsePacket: public ResponsePacket
    {
    public:
        ErrorResponsePacket()
            : ResponsePacket(std::vector<unsigned char>{'E', '0', '1'})
        {};
    };
}
