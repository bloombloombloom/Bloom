#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/Protocols/RiscVDebug/TriggerModule/TriggerModule.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebug::TriggerModule::Registers
{
    /**
     * TODO: Given the single, full width bit field, is this struct really necessary? Review.
     */
    struct TriggerSelect
    {
        TriggerIndex index;

        constexpr explicit TriggerSelect(TriggerIndex index)
            : index(index)
        {}

        [[nodiscard]] constexpr RegisterValue value() const {
            return static_cast<RegisterValue>(this->index);
        }
    };
}
