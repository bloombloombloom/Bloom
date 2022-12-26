#pragma once

#include "InsightWorkerTask.hpp"

#include "src/Targets/TargetDescriptor.hpp"

namespace Bloom
{
    class GetTargetDescriptor: public InsightWorkerTask
    {
        Q_OBJECT

    public:
        GetTargetDescriptor() = default;

        TaskGroups getTaskGroups() const override {
            return TaskGroups({
                TaskGroup::USES_TARGET_CONTROLLER,
            });
        };

    signals:
        void targetDescriptor(Targets::TargetDescriptor targetDescriptor);

    protected:
        void run(Services::TargetControllerService& targetControllerService) override;
    };
}
