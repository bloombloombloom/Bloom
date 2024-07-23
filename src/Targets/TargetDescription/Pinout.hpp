#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "Pin.hpp"

#include "src/Targets/TargetPinoutDescriptor.hpp"

namespace Targets::TargetDescription
{
    struct Pinout
    {
        std::string key;
        std::string name;
        ::Targets::TargetPinoutType type;
        std::optional<std::string> function;
        std::vector<Pin> pins;

        Pinout(
            const std::string& key,
            const std::string& name,
            ::Targets::TargetPinoutType type,
            const std::optional<std::string>& function,
            const std::vector<Pin>& pins
        )
            : key(key)
            , name(name)
            , type(type)
            , function(function)
            , pins(pins)
        {}
    };
}
