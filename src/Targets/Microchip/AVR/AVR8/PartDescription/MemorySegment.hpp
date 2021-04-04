#pragma once

#include <cstdint>
#include <QDomElement>

#include "src/Helpers/BiMap.hpp"

namespace Bloom::Targets::Microchip::Avr::Avr8Bit::PartDescription
{
    enum MemorySegmentType {
        REGISTERS,
        IO,
        EEPROM,
        RAM,
        FLASH,
        SIGNATURES,
        FUSES,
        LOCKBITS,
        OSCCAL,
    };

    struct MemorySegment {
        std::string name;
        MemorySegmentType type;
        std::uint32_t startAddress;
        std::uint32_t size;
        std::optional<std::uint16_t> pageSize;

        /**
         * Mapping of all known memory segment types by their name. Any memory segments belonging to a type
         * not defined in here should be ignored.
         */
        static const inline BiMap<std::string, MemorySegmentType> typesMappedByName = {
            {"regs", MemorySegmentType::REGISTERS},
            {"io", MemorySegmentType::IO},
            {"eeprom", MemorySegmentType::EEPROM},
            {"ram", MemorySegmentType::RAM},
            {"flash", MemorySegmentType::FLASH},
            {"signatures", MemorySegmentType::SIGNATURES},
            {"fuses", MemorySegmentType::FUSES},
            {"lockbits", MemorySegmentType::LOCKBITS},
            {"osccal", MemorySegmentType::OSCCAL},
        };
    };
}
