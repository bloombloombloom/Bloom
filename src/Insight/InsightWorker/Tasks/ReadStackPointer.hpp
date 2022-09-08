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

        TaskGroups getTaskGroups() const override {
            return TaskGroups({
                TaskGroup::USES_TARGET_CONTROLLER,
            });
        };

    signals:
        void stackPointerRead(Targets::TargetStackPointer stackPointer);

    protected:
        void run(TargetController::TargetControllerConsole& targetControllerConsole) override;
    };
}
