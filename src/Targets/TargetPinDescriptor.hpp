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

        bool operator == (const TargetPinDescriptor& pinDescriptor) const {
            return this->number == pinDescriptor.number
                && this->supportsGpio == pinDescriptor.supportsGpio
                && this->name == pinDescriptor.name
                && this->padName == pinDescriptor.padName
                && this->functions == pinDescriptor.functions
                && this->type == pinDescriptor.type
            ;
        }
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
