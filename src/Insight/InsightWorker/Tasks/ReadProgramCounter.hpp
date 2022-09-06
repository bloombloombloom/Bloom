#pragma once

#include "InsightWorkerTask.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace Bloom
{
    class ReadProgramCounter: public InsightWorkerTask
    {
        Q_OBJECT

    public:
        ReadProgramCounter() = default;

    signals:
        void programCounterRead(Targets::TargetProgramCounter programCounter);

    protected:
        void run(TargetController::TargetControllerConsole& targetControllerConsole) override;
    };
}
