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

    signals:
        void targetDescriptor(Targets::TargetDescriptor targetDescriptor);

    protected:
        void run(TargetController::TargetControllerConsole& targetControllerConsole) override;
    };
}
