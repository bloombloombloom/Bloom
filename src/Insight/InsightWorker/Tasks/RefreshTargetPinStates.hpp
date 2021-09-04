#pragma once

#include "InsightWorkerTask.hpp"
#include "src/Targets/TargetVariant.hpp"
#include "src/Targets/TargetPinDescriptor.hpp"

namespace Bloom
{
    class RefreshTargetPinStates: public InsightWorkerTask
    {
    Q_OBJECT
    private:
        int variantId;

    protected:
        void run(TargetControllerConsole& targetControllerConsole) override;

    public:
        RefreshTargetPinStates(int variantId): InsightWorkerTask(), variantId(variantId) {}

    signals:
        void targetPinStatesRetrieved(Bloom::Targets::TargetPinStateMappingType pinStatesByNumber);
    };
}
