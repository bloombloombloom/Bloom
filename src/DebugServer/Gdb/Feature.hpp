#pragma once

#include "src/Helpers/BiMap.hpp"

namespace DebugServer::Gdb
{
    enum class Feature: int
    {
        SOFTWARE_BREAKPOINTS,
        HARDWARE_BREAKPOINTS,
        PACKET_SIZE,
        MEMORY_MAP_READ,
        VCONT_ACTIONS_QUERY,
        NO_ACK_MODE,
    };

    static inline BiMap<Feature, std::string> getGdbFeatureToNameMapping() {
        return BiMap<Feature, std::string>{
            {Feature::HARDWARE_BREAKPOINTS, "hwbreak"},
            {Feature::SOFTWARE_BREAKPOINTS, "swbreak"},
            {Feature::PACKET_SIZE, "PacketSize"},
            {Feature::MEMORY_MAP_READ, "qXfer:memory-map:read"},
            {Feature::VCONT_ACTIONS_QUERY, "vContSupported"},
            {Feature::NO_ACK_MODE, "QStartNoAckMode"},
        };
    }
}
