#pragma once

#include "InsightWorkerTask.hpp"
#include "src/Targets/TargetRegisterDescriptor.hpp"

class WriteTargetRegister: public InsightWorkerTask
{
    Q_OBJECT

public:
    explicit WriteTargetRegister(const Targets::TargetRegisterDescriptorAndValuePair& registerPair);
    [[nodiscard]] QString brief() const override;
    [[nodiscard]] TaskGroups taskGroups() const override;

protected:
    void run(Services::TargetControllerService& targetControllerService) override;

private:
    Targets::TargetRegisterDescriptorAndValuePair registerPair;
};
