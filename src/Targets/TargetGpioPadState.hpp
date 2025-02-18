#pragma once

#include <cstdint>
#include <QMetaType>

#include "TargetPadDescriptor.hpp"

#include "src/Helpers/Pair.hpp"

namespace Targets
{
    struct TargetGpioPadState
    {
        enum class State: std::uint8_t
        {
            HIGH,
            LOW,
            UNKNOWN,
        };

        enum class DataDirection: std::uint8_t
        {
            INPUT,
            OUTPUT,
            UNKNOWN,
        };

        bool disabled = false;
        State value = State::UNKNOWN;
        DataDirection direction = DataDirection::UNKNOWN;

        bool operator == (const TargetGpioPadState& other) const {
            return this->value == other.value && this->direction == other.direction;
        }

        bool operator != (const TargetGpioPadState& other) const {
            return !(*this == other);
        }
    };

    using TargetGpioPadDescriptorAndStatePair = Pair<const TargetPadDescriptor&, TargetGpioPadState>;
    using TargetGpioPadDescriptorAndStatePairs = std::vector<TargetGpioPadDescriptorAndStatePair>;
}

Q_DECLARE_METATYPE(Targets::TargetGpioPadState)
