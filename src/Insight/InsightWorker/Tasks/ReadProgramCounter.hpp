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

        TaskGroups getTaskGroups() const override {
            return TaskGroups({
                TaskGroup::USES_TARGET_CONTROLLER,
            });
        };

    signals:
        void programCounterRead(Targets::TargetProgramCounter programCounter);

    protected:
        void run(TargetController::TargetControllerConsole& targetControllerConsole) override;
    };
}
