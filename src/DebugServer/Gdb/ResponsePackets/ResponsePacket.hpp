#pragma once

#include <vector>
#include <string>

#include "src/DebugServer/Gdb/Packet.hpp"

namespace DebugServer::Gdb::ResponsePackets
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

        explicit ResponsePacket(const std::string& data) {
            this->data = std::vector<unsigned char>{data.begin(), data.end()};
        }

        /**
         * Generates a raw packet.
         *
         * @return
         */
        [[nodiscard]] RawPacket toRawPacket() const;
    };
}
