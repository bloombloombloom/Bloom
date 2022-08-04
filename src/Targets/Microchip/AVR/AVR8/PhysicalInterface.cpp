#include "PhysicalInterface.hpp"

namespace Bloom::Targets::Microchip::Avr::Avr8Bit
{
    std::map<PhysicalInterface, std::string> getPhysicalInterfaceNames() {
        return std::map<PhysicalInterface, std::string>({
            {PhysicalInterface::ISP, "ISP"},
            {PhysicalInterface::DEBUG_WIRE, "debugWire"},
            {PhysicalInterface::PDI, "PDI"},
            {PhysicalInterface::JTAG, "JTAG"},
            {PhysicalInterface::UPDI, "UPDI"},
        });
    }
}
