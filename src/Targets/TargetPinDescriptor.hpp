#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <optional>
#include <map>

namespace Bloom::Targets
{
    enum class TargetPinType: int
    {
        UNKNOWN,
        GPIO,
        GND,
        VCC,
    };

    struct TargetPinDescriptor
    {
        int number;
        bool supportsGpio = false;
        std::string name;
        std::string padName;
        std::vector<std::string> functions;

        TargetPinType type = TargetPinType::UNKNOWN;
    };

    struct TargetPinState
    {
        enum class IoState: int
        {
            HIGH,
            LOW,
        };

        enum class IoDirection: int
        {
            INPUT,
            OUTPUT,
        };

        std::optional<IoState> ioState;
        std::optional<IoDirection> ioDirection;
    };

    using TargetPinStateMappingType = std::map<int, Bloom::Targets::TargetPinState>;
}

Q_DECLARE_METATYPE(Bloom::Targets::TargetPinDescriptor)
Q_DECLARE_METATYPE(Bloom::Targets::TargetPinState)
Q_DECLARE_METATYPE(Bloom::Targets::TargetPinStateMappingType)
