#pragma once

#include <cstdint>

#include "InsightWorkerTask.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Helpers/EnumToStringMappings.hpp"

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
            , excludedAddressRanges(excludedAddressRanges)
        {}

        QString brief() const override {
            return "Reading target " + EnumToStringMappings::targetMemoryTypes.at(this->memoryType).toUpper();
        }

        TaskGroups taskGroups() const override {
            return TaskGroups({
                TaskGroup::USES_TARGET_CONTROLLER,
            });
        };

    signals:
        void targetMemoryRead(Targets::TargetMemoryBuffer buffer);

    protected:
        void run(Services::TargetControllerService& targetControllerService) override;

    private:
        Targets::TargetMemoryType memoryType;
        Targets::TargetMemoryAddress startAddress;
        Targets::TargetMemorySize size;
        std::set<Targets::TargetMemoryAddressRange> excludedAddressRanges;
    };
}
