#pragma once

#include <cstdint>

#include "src/Targets/TargetRegisterDescriptor.hpp"

#include "RiscVGeneric.hpp"

namespace Targets::RiscV
{
    struct RiscVRegisterDescriptor: public ::Targets::TargetRegisterDescriptor
    {
        RegisterNumber number;

        RiscVRegisterDescriptor(
            TargetRegisterType type,
            RegisterNumber number,
            TargetMemorySize size,
            TargetMemoryType memoryType,
            std::optional<std::string> name,
            std::optional<std::string> groupName,
            std::optional<std::string> description,
            TargetRegisterAccess access
        )
            : ::Targets::TargetRegisterDescriptor(
                type,
                std::nullopt,
                size,
                memoryType,
                name,
                groupName,
                description,
                access
            )
            , number(number)
        {}
    };
}
