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
        ReadTargetMemory(Targets::TargetMemoryType memoryType, std::uint32_t startAddress, std::uint32_t size):
        InsightWorkerTask(), memoryType(memoryType), startAddress(startAddress), size(size) {}

    signals:
        void targetMemoryRead(Targets::TargetMemoryBuffer buffer);

    protected:
        void run(TargetControllerConsole& targetControllerConsole) override;

    private:
        Targets::TargetMemoryType memoryType;
        std::uint32_t startAddress;
        std::uint32_t size;
    };
}
