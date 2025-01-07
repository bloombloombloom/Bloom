#pragma once

#include <cstdint>
#include <optional>

#include "src/Targets/Microchip/Avr8/TargetDescriptionFile.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::Parameters::Avr8Generic
{
    /**
     * EDBG parameters for debugWIRE and JTAG AVR targets.
     *
     * See Microchip's "EDBG-based Tools Protocols" document for more on these parameters.
     */
    struct DebugWireJtagParameters
    {
        std::uint16_t flashPageSize;
        std::uint32_t flashSize;
        std::uint32_t flashStartWordAddress;
        std::optional<std::uint32_t> bootSectionStartWordAddress;
        std::uint16_t ramStartAddress;
        std::uint16_t eepromSize;
        std::uint8_t eepromPageSize;
        std::uint8_t ocdRevision;
        std::uint8_t ocdDataRegisterAddress;
        std::uint8_t eearAddressHigh;
        std::uint8_t eearAddressLow;
        std::uint8_t eedrAddress;
        std::uint8_t eecrAddress;
        std::uint8_t spmcrAddress;
        std::uint8_t osccalAddress;

        DebugWireJtagParameters(const Targets::Microchip::Avr8::TargetDescriptionFile& targetDescriptionFile);
    };
}
