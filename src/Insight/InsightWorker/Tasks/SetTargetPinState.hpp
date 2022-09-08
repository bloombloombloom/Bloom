#pragma once

#include "InsightWorkerTask.hpp"
#include "src/Targets/TargetPinDescriptor.hpp"

namespace Bloom
{
    class SetTargetPinState: public InsightWorkerTask
    {
        Q_OBJECT

    public:
        SetTargetPinState(const Targets::TargetPinDescriptor& pinDescriptor, const Targets::TargetPinState& pinState)
            : pinDescriptor(pinDescriptor)
            , pinState(pinState)
        {}

        TaskGroups getTaskGroups() const override {
            return TaskGroups({
                TaskGroup::USES_TARGET_CONTROLLER,
            });
        };

    protected:
        void run(TargetController::TargetControllerConsole& targetControllerConsole) override;

    private:
        Targets::TargetPinDescriptor pinDescriptor;
        Targets::TargetPinState pinState;
    };
}
