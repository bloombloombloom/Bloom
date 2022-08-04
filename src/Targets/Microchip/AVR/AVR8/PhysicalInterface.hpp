#pragma once

#include <cstdint>
#include <map>
#include <string>

namespace Bloom::Targets::Microchip::Avr::Avr8Bit
{
    enum class PhysicalInterface: std::uint8_t
    {
        ISP,
        JTAG,
        DEBUG_WIRE,
        PDI,
        UPDI,
    };

    /**
     * Returns a mapping of physical interfaces to their marketing name.
     *
     * @return
     */
    std::map<PhysicalInterface, std::string> getPhysicalInterfaceNames();
}
