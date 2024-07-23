#pragma once

#include <cstdint>
#include <QMetaType>

#include "TargetPinDescriptor.hpp"

namespace Targets
{
    struct TargetGpioPinState
    {
        enum class State: std::uint8_t
        {
            HIGH,
            LOW,
        };

        enum class DataDirection: std::uint8_t
        {
            INPUT,
            OUTPUT,
        };

        State value;
        DataDirection direction;

        TargetGpioPinState(
            State value,
            DataDirection direction
        )
            : value(value)
            , direction(direction)
        {}
    };

    using TargetGpioPinDescriptorAndStatePair = Pair<const TargetPinDescriptor&, TargetGpioPinState>;
    using TargetGpioPinDescriptorAndStatePairs = std::vector<TargetGpioPinDescriptorAndStatePair>;
}

Q_DECLARE_METATYPE(Targets::TargetGpioPinState)
