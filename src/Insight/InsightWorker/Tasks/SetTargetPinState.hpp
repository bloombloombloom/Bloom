#pragma once

#include "InsightWorkerTask.hpp"
#include "src/Targets/TargetPinDescriptor.hpp"

namespace Bloom
{
    class SetTargetPinState: public InsightWorkerTask
    {
    Q_OBJECT
    private:
        Targets::TargetPinDescriptor pinDescriptor;
        Targets::TargetPinState pinState;

    protected:
        void run(TargetControllerConsole& targetControllerConsole) override;

    public:
        SetTargetPinState(const Targets::TargetPinDescriptor& pinDescriptor, const Targets::TargetPinState& pinState):
        InsightWorkerTask(), pinDescriptor(pinDescriptor), pinState(pinState) {}
    };
}
