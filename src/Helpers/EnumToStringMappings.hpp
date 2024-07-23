#pragma once

#include <QString>

#include "BiMap.hpp"

#include "src/Targets/TargetMemorySegmentType.hpp"

class EnumToStringMappings
{
public:
    static const inline BiMap<Targets::TargetMemorySegmentType, QString> targetMemorySegmentTypes = {
        {Targets::TargetMemorySegmentType::RAM, "ram"},
        {Targets::TargetMemorySegmentType::EEPROM, "eeprom"},
        {Targets::TargetMemorySegmentType::FLASH, "flash"},
    };

    static const inline BiMap<Targets::TargetMemoryEndianness, QString> targetMemoryEndianness = {
        {Targets::TargetMemoryEndianness::LITTLE, "little"},
        {Targets::TargetMemoryEndianness::BIG, "big"},
    };
};
