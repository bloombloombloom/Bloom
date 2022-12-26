#pragma once

#include "InsightWorkerTask.hpp"
#include "src/Targets/TargetRegister.hpp"

namespace Bloom
{
    class ReadTargetRegisters: public InsightWorkerTask
    {
        Q_OBJECT

    public:
        explicit ReadTargetRegisters(const Targets::TargetRegisterDescriptors& descriptors)
            : descriptors(descriptors)
        {}

        TaskGroups getTaskGroups() const override {
            return TaskGroups({
                TaskGroup::USES_TARGET_CONTROLLER,
            });
        };

    signals:
        void targetRegistersRead(Targets::TargetRegisters registers);

    protected:
        void run(Services::TargetControllerService& targetControllerService) override;

    private:
        Targets::TargetRegisterDescriptors descriptors;
    };
}
