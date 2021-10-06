#pragma once

#include "InsightWorkerTask.hpp"
#include "src/Targets/TargetRegister.hpp"

namespace Bloom
{
    class WriteTargetRegister: public InsightWorkerTask
    {
        Q_OBJECT

    public:
        WriteTargetRegister(const Targets::TargetRegister& targetRegister):
        InsightWorkerTask(), targetRegister(targetRegister) {}

    protected:
        void run(TargetControllerConsole& targetControllerConsole) override;

    private:
        Targets::TargetRegister targetRegister;
    };
}
