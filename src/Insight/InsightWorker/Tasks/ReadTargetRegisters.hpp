#pragma once

#include "InsightWorkerTask.hpp"
#include "src/Targets/TargetRegisterDescriptor.hpp"

class ReadTargetRegisters: public InsightWorkerTask
{
    Q_OBJECT

public:
    explicit ReadTargetRegisters(const Targets::TargetRegisterDescriptors& descriptors);
    [[nodiscard]] QString brief() const override;
    [[nodiscard]] TaskGroups taskGroups() const override;

signals:
    void targetRegistersRead(Targets::TargetRegisterDescriptorAndValuePairs registers);

protected:
    void run(Services::TargetControllerService& targetControllerService) override;

private:
    Targets::TargetRegisterDescriptors descriptors;
};
