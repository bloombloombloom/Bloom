#pragma once

#include "ResponsePacket.hpp"

namespace Bloom::DebugServers::Gdb::ResponsePackets
{
    /**
     * Error response packet expected by the GDB client, to indicate an error, in response to certain commands.
     */
    class ErrorResponsePacket: public ResponsePacket
    {
    public:
        ErrorResponsePacket() = default;

        [[nodiscard]] std::vector<unsigned char> getData() const override {
            return {'E', '0', '1'};
        }
    };
}
