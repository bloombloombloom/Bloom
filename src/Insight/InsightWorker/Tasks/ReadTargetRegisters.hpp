#pragma once

#include "InsightWorkerTask.hpp"
#include "src/Targets/TargetRegister.hpp"

namespace Bloom
{
    class ReadTargetRegisters: public InsightWorkerTask
    {
    Q_OBJECT
    private:
        Targets::TargetRegisterDescriptors descriptors;

    protected:
        void run(TargetControllerConsole& targetControllerConsole) override;

    public:
        ReadTargetRegisters(const Targets::TargetRegisterDescriptors& descriptors):
        InsightWorkerTask(), descriptors(descriptors) {}

    signals:
        void targetRegistersRead(Targets::TargetRegisters registers);
    };
}
