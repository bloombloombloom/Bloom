#pragma once

#include "src/Helpers/BiMap.hpp"

namespace Bloom::DebugServers::Gdb
{
    enum class StopReason: int
    {
        SOFTWARE_BREAKPOINT = 0,
        HARDWARE_BREAKPOINT = 1,
    };

    static inline BiMap<StopReason, std::string> getStopReasonToNameMapping() {
        return BiMap<StopReason, std::string>({
            {StopReason::HARDWARE_BREAKPOINT, "hwbreak"},
            {StopReason::SOFTWARE_BREAKPOINT, "swbreak"},
        });
    }
}
