#pragma once

#include <cstdint>
#include <string>
#include <map>
#include <optional>

#include "BitField.hpp"

namespace Targets::TargetDescription
{
    struct Register
    {
        std::string name;
        std::optional<std::string> caption;
        std::uint16_t offset;
        std::uint16_t size;
        std::optional<std::string> readWriteAccess;
        std::map<std::string, BitField> bitFieldsMappedByName;
    };

    struct RegisterGroup
    {
        std::string name;
        std::optional<std::string> moduleName;
        std::optional<std::uint16_t> offset;
        std::optional<std::string> addressSpaceId;
        std::map<std::string, Register> registersMappedByName;
    };
}
