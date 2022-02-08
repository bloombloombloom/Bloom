#pragma once

#include "InsightWorkerTask.hpp"
#include "src/Targets/TargetPinDescriptor.hpp"

namespace Bloom
{
    class SetTargetPinState: public InsightWorkerTask
    {
        Q_OBJECT

    public:
        SetTargetPinState(const Targets::TargetPinDescriptor& pinDescriptor, const Targets::TargetPinState& pinState):
        pinDescriptor(pinDescriptor), pinState(pinState) {}

    protected:
        void run(TargetControllerConsole& targetControllerConsole) override;

    private:
        Targets::TargetPinDescriptor pinDescriptor;
        Targets::TargetPinState pinState;
    };
}
