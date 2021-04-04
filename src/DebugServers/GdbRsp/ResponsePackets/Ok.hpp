#pragma once

#include <set>

#include "ResponsePacket.hpp"

namespace Bloom::DebugServers::Gdb {
    enum class Feature;
}

namespace Bloom::DebugServers::Gdb::ResponsePackets
{
    /**
     * OK response packet expected by the GDB client, in response to certain commands.
     */
    class Ok: public ResponsePacket
    {
    public:
        Ok() = default;

        std::vector<unsigned char> getData() const override {
            return {'O', 'K'};
        }
    };
}
