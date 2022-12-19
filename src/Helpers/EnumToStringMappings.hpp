#pragma once

#include <QString>

#include "BiMap.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace Bloom
{
    class EnumToStringMappings
    {
    public:
        static const inline BiMap<Targets::TargetMemoryType, QString> targetMemoryTypes = {
            {Targets::TargetMemoryType::RAM, "ram"},
            {Targets::TargetMemoryType::EEPROM, "eeprom"},
            {Targets::TargetMemoryType::FLASH, "flash"},
            {Targets::TargetMemoryType::OTHER, "other"},
        };

        static const inline BiMap<Targets::TargetMemoryEndianness, QString> targetMemoryEndianness = {
            {Targets::TargetMemoryEndianness::LITTLE, "little"},
            {Targets::TargetMemoryEndianness::BIG, "big"},
        };
    };
}
