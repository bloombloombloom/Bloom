#pragma once

#include <cstdint>
#include <map>
#include <string>

namespace Targets
{
    enum class TargetPhysicalInterface: std::uint8_t
    {
        ISP,
        JTAG,
        DEBUG_WIRE,
        PDI,
        UPDI,
        SDI,
    };

    /**
     * Returns a mapping of physical interfaces to their marketing name.
     *
     * @return
     */
    std::map<TargetPhysicalInterface, std::string> getPhysicalInterfaceNames();
}
