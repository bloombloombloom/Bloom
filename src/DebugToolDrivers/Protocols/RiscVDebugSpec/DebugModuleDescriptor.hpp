#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "Common.hpp"
#include "DebugModule/DebugModule.hpp"
#include "TriggerModule/TriggerModule.hpp"
#include "TriggerModule/TriggerDescriptor.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebugSpec
{
    struct DebugModuleDescriptor
    {
        std::vector<DebugModule::HartIndex> hartIndices;

        std::unordered_set<DebugModule::MemoryAccessStrategy> memoryAccessStrategies;
        std::uint8_t programBufferSize = 0;

        std::unordered_map<TriggerModule::TriggerIndex, TriggerModule::TriggerDescriptor> triggerDescriptorsByIndex;
    };
}
