#pragma once

#include "InsightWorkerTask.hpp"
#include "src/Targets/TargetRegister.hpp"

namespace Bloom
{
    class ReadTargetRegisters: public InsightWorkerTask
    {
        Q_OBJECT

    public:
        explicit ReadTargetRegisters(const Targets::TargetRegisterDescriptors& descriptors): descriptors(descriptors) {}

    signals:
        void targetRegistersRead(Targets::TargetRegisters registers);

    protected:
        void run(TargetController::TargetControllerConsole& targetControllerConsole) override;

    private:
        Targets::TargetRegisterDescriptors descriptors;
    };
}
