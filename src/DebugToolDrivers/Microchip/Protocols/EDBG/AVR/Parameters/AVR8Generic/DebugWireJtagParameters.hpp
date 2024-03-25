#pragma once

#include <cstdint>
#include <optional>

#include "src/Targets/Microchip/AVR/AVR8/TargetDescriptionFile.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::Parameters::Avr8Generic
{
    /**
     * EDBG parameters for debugWire and JTAG AVR targets.
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
        std::uint8_t ocdDataRegister;
        std::uint8_t eepromAddressRegisterHigh;
        std::uint8_t eepromAddressRegisterLow;
        std::uint8_t eepromDataRegisterAddress;
        std::uint8_t eepromControlRegisterAddress;
        std::uint8_t spmcRegisterStartAddress;
        std::uint8_t osccalAddress;

        DebugWireJtagParameters(const Targets::Microchip::Avr::Avr8Bit::TargetDescriptionFile& targetDescriptionFile);
    };
}
