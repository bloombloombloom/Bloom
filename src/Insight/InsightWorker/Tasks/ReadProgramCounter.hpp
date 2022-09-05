#pragma once

#include "InsightWorkerTask.hpp"

namespace Bloom
{
    class ReadProgramCounter: public InsightWorkerTask
    {
        Q_OBJECT

    public:
        ReadProgramCounter() = default;

    signals:
        void programCounterRead(std::uint32_t programCounter);

    protected:
        void run(TargetController::TargetControllerConsole& targetControllerConsole) override;
    };
}
