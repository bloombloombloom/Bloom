#pragma once

#include "src/Helpers/BiMap.hpp"

namespace Bloom::DebugServer::Gdb
{
    enum class Feature: int
    {
        SOFTWARE_BREAKPOINTS,
        HARDWARE_BREAKPOINTS,
        PACKET_SIZE,
        MEMORY_MAP_READ,
    };

    static inline BiMap<Feature, std::string> getGdbFeatureToNameMapping() {
        return BiMap<Feature, std::string>{
            {Feature::HARDWARE_BREAKPOINTS, "hwbreak"},
            {Feature::SOFTWARE_BREAKPOINTS, "swbreak"},
            {Feature::PACKET_SIZE, "PacketSize"},
            {Feature::MEMORY_MAP_READ, "qXfer:memory-map:read"},
        };
    }
}
