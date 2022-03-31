#pragma once

#include <vector>
#include <memory>

#include "src/DebugServer/GdbRsp/Packet.hpp"

namespace Bloom::DebugServer::Gdb::ResponsePackets
{
    /**
     * Upon receiving a CommandPacket from the connected GDB RSP client, the server is expected to respond with a
     * response packet.
     */
    class ResponsePacket: public Packet
    {
    public:
        ResponsePacket() = default;
        explicit ResponsePacket(const std::vector<unsigned char>& data) {
            this->data = data;
        }
    };
}
