#pragma once

#include "InsightWorkerTask.hpp"
#include "src/Targets/TargetRegister.hpp"

namespace Bloom
{
    class WriteTargetRegister: public InsightWorkerTask
    {
    Q_OBJECT
    private:
        Targets::TargetRegister targetRegister;

    protected:
        void run(TargetControllerConsole& targetControllerConsole) override;

    public:
        WriteTargetRegister(const Targets::TargetRegister& targetRegister):
        InsightWorkerTask(), targetRegister(targetRegister) {}
    };
}
