#pragma once

#include <cstdint>

#include "src/Targets/Microchip/AVR8/TargetDescriptionFile.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::Parameters::Avr8Generic
{
    /**
     * EDBG parameters for PDI-enabled (XMega) AVR targets.
     *
     * See Microchip's "EDBG-based Tools Protocols" document for more on these parameters.
     */
    struct PdiParameters
    {
        std::uint32_t appSectionPdiOffset;
        std::uint32_t bootSectionPdiOffset;
        std::uint32_t eepromPdiOffset;
        std::uint32_t fuseRegistersPdiOffset;
        std::uint32_t lockRegistersPdiOffset;
        std::uint32_t userSignaturesPdiOffset;
        std::uint32_t prodSignaturesPdiOffset;
        std::uint32_t ramPdiOffset;
        std::uint32_t appSectionSize;
        std::uint16_t bootSectionSize;
        std::uint16_t flashPageSize;
        std::uint16_t eepromSize;
        std::uint8_t eepromPageSize;
        std::uint16_t nvmModuleBaseAddress;
        std::uint16_t signaturesPdiOffset;

        PdiParameters(const Targets::Microchip::Avr8::TargetDescriptionFile& targetDescriptionFile);
    };
}
