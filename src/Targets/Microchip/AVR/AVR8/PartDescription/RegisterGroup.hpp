#pragma once

#include <cstdint>
#include <map>
#include <optional>

namespace Bloom::Targets::Microchip::Avr::Avr8Bit::PartDescription
{
    struct Register {
        std::string name;
        std::uint16_t offset;
        std::uint16_t size;
    };

    struct RegisterGroup {
        std::string name;
        std::optional<std::uint16_t> offset;
        std::map<std::string, Register> registersMappedByName;
    };
}
