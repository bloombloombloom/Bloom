#pragma once

#include "InsightWorkerTask.hpp"

#include "src/Targets/TargetState.hpp"

namespace Bloom
{
    class GetTargetState: public InsightWorkerTask
    {
        Q_OBJECT

    public:
        GetTargetState() = default;

        TaskGroups getTaskGroups() const override {
            return TaskGroups({
                TaskGroup::USES_TARGET_CONTROLLER,
            });
        };

    signals:
        void targetState(Targets::TargetState state);

    protected:
        void run(TargetController::TargetControllerConsole& targetControllerConsole) override;
    };
}
