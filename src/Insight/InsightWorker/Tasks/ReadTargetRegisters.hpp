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

        QString brief() const override {
            return "Reading target registers";
        }

        TaskGroups taskGroups() const override {
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
