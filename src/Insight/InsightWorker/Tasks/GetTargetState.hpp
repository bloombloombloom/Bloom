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

    signals:
        void targetState(Targets::TargetState state);

    protected:
        void run(TargetController::TargetControllerConsole& targetControllerConsole) override;
    };
}
