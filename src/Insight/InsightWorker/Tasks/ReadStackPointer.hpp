#pragma once

#include "InsightWorkerTask.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace Bloom
{
    class ReadStackPointer: public InsightWorkerTask
    {
        Q_OBJECT

    public:
        ReadStackPointer() = default;

    signals:
        void stackPointerRead(Targets::TargetStackPointer stackPointer);

    protected:
        void run(TargetController::TargetControllerConsole& targetControllerConsole) override;
    };
}
