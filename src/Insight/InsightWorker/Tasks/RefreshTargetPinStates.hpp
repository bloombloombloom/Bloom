#pragma once

#include "InsightWorkerTask.hpp"
#include "src/Targets/TargetVariant.hpp"
#include "src/Targets/TargetPinDescriptor.hpp"

namespace Bloom
{
    class RefreshTargetPinStates: public InsightWorkerTask
    {
        Q_OBJECT

    public:
        explicit RefreshTargetPinStates(int variantId)
            : variantId(variantId)
        {}

        TaskGroups getTaskGroups() const override {
            return TaskGroups({
                TaskGroup::USES_TARGET_CONTROLLER,
            });
        };

    signals:
        void targetPinStatesRetrieved(Bloom::Targets::TargetPinStateMappingType pinStatesByNumber);

    protected:
        void run(TargetController::TargetControllerConsole& targetControllerConsole) override;

    private:
        int variantId;
    };
}
