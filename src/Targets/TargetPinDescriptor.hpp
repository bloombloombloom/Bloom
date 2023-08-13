#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <optional>
#include <map>
#include <QMetaType>

namespace Targets
{
    enum class TargetPinType: std::uint8_t
    {
        UNKNOWN,
        GPIO,
        GND,
        VCC,
    };

    struct TargetPinDescriptor
    {
        int number;
        int variantId;
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
        enum class IoState: std::uint8_t
        {
            HIGH,
            LOW,
        };

        enum class IoDirection: std::uint8_t
        {
            INPUT,
            OUTPUT,
        };

        std::optional<IoState> ioState;
        std::optional<IoDirection> ioDirection;
    };

    using TargetPinStateMapping = std::map<int, Targets::TargetPinState>;
}

Q_DECLARE_METATYPE(Targets::TargetPinDescriptor)
Q_DECLARE_METATYPE(Targets::TargetPinState)
Q_DECLARE_METATYPE(Targets::TargetPinStateMapping)
