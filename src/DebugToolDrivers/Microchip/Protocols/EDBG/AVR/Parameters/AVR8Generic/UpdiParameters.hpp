#pragma once

#include <cstdint>

#include "src/Targets/Microchip/AVR8/TargetDescriptionFile.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::Parameters::Avr8Generic
{
    /**
     * EDBG parameters for UPDI-enabled AVR targets.
     *
     * See Microchip's "EDBG-based Tools Protocols" document for more on these parameters.
     * BTW that document seems to be a little outdated. It doesn't list most of these parameters. I discovered the
     * unlisted ones by looking at other open-source codebases and reverse engineering.
     */
    struct UpdiParameters
    {
        std::uint32_t programMemoryStartAddress;
        std::uint16_t flashPageSize;
        std::uint8_t eepromPageSize;
        std::uint16_t nvmModuleBaseAddress;
        std::uint16_t ocdModuleAddress;
        std::uint32_t flashSize;
        std::uint16_t eepromSize;
        std::uint16_t fuseSegmentSize;
        std::uint16_t eepromStartAddress;
        std::uint16_t signatureSegmentStartAddress;
        std::uint16_t fuseSegmentStartAddress;
        std::uint16_t lockbitSegmentStartAddress;

        UpdiParameters(const Targets::Microchip::Avr8::TargetDescriptionFile& targetDescriptionFile);
    };
}
