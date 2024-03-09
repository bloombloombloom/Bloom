#pragma once

#include "InsightWorkerTask.hpp"
#include "src/Targets/TargetRegisterDescriptor.hpp"
#include "src/Targets/TargetRegister.hpp"

class ReadTargetRegisters: public InsightWorkerTask
{
    Q_OBJECT

public:
    explicit ReadTargetRegisters(const Targets::TargetRegisterDescriptorIds& descriptorIds)
        : descriptorIds(descriptorIds)
    {}

    QString brief() const override {
        return "Reading " + QString::number(this->descriptorIds.size()) + " target register(s)";
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
    Targets::TargetRegisterDescriptorIds descriptorIds;
};
