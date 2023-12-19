#pragma once

#include <cstdint>
#include <optional>
#include <QDomElement>

#include "src/Helpers/BiMap.hpp"

namespace Targets::TargetDescription
{
    enum class MemorySegmentType: std::uint8_t
    {
        ALIASED,
        REGISTERS,
        EEPROM,
        FLASH,
        FUSES,
        IO,
        RAM,
        LOCKBITS,
        OSCCAL,
        SIGNATURES,
        USER_SIGNATURES,
    };

    struct MemorySegment
    {
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
            {"aliased", MemorySegmentType::ALIASED},
            {"regs", MemorySegmentType::REGISTERS},
            {"eeprom", MemorySegmentType::EEPROM},
            {"flash", MemorySegmentType::FLASH},
            {"fuses", MemorySegmentType::FUSES},
            {"io", MemorySegmentType::IO},
            {"ram", MemorySegmentType::RAM},
            {"lockbits", MemorySegmentType::LOCKBITS},
            {"osccal", MemorySegmentType::OSCCAL},
            {"signatures", MemorySegmentType::SIGNATURES},
            {"user_signatures", MemorySegmentType::USER_SIGNATURES},
        };
    };
}
