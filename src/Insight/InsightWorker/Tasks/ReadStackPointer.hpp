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

        TaskGroups taskGroups() const override {
            return TaskGroups({
                TaskGroup::USES_TARGET_CONTROLLER,
            });
        };

    signals:
        void stackPointerRead(Targets::TargetStackPointer stackPointer);

    protected:
        void run(Services::TargetControllerService& targetControllerService) override;
    };
}
