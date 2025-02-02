#pragma once

#include <cstdint>
#include <set>

#include "TriggerModule.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebug::TriggerModule
{
    struct TriggerDescriptor
    {
        TriggerIndex index;
        std::set<TriggerType> supportedTypes;

        TriggerDescriptor(TriggerIndex index, const std::set<TriggerType>& supportedTypes)
            : index(index)
            , supportedTypes(supportedTypes)
        {}
    };
}
