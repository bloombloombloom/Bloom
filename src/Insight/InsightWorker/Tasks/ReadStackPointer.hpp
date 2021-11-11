#pragma once

#include "InsightWorkerTask.hpp"

namespace Bloom
{
    class ReadStackPointer: public InsightWorkerTask
    {
        Q_OBJECT

    public:
        ReadStackPointer(): InsightWorkerTask() {}

    signals:
        void stackPointerRead(std::uint32_t stackPointer);

    protected:
        void run(TargetControllerConsole& targetControllerConsole) override;
    };
}
