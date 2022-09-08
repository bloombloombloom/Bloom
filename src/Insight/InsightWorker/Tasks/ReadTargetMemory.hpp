#pragma once

#include <cstdint>

#include "InsightWorkerTask.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace Bloom
{
    class ReadTargetMemory: public InsightWorkerTask
    {
        Q_OBJECT

    public:
        ReadTargetMemory(
            Targets::TargetMemoryType memoryType,
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemorySize size,
            const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges = {}
        )
            : memoryType(memoryType)
            , startAddress(startAddress)
            , size(size)
            , excludedAddressRanges(excludedAddressRanges) {}

        TaskGroups getTaskGroups() const override {
            return TaskGroups({
                TaskGroup::USES_TARGET_CONTROLLER,
            });
        };

    signals:
        void targetMemoryRead(Targets::TargetMemoryBuffer buffer);

    protected:
        void run(TargetController::TargetControllerConsole& targetControllerConsole) override;

    private:
        Targets::TargetMemoryType memoryType;
        Targets::TargetMemoryAddress startAddress;
        Targets::TargetMemorySize size;
        std::set<Targets::TargetMemoryAddressRange> excludedAddressRanges;
    };
}
