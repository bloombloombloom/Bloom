#pragma once

#include "InsightWorkerTask.hpp"

#include "src/Targets/TargetPadDescriptor.hpp"
#include "src/Targets/TargetGpioPadState.hpp"

class ReadTargetGpioPadStates: public InsightWorkerTask
{
    Q_OBJECT

public:
    explicit ReadTargetGpioPadStates(const Targets::TargetPadDescriptors& padDescriptors);
    [[nodiscard]] QString brief() const override;
    [[nodiscard]] TaskGroups taskGroups() const override;

signals:
    void targetGpioPadStatesRead(Targets::TargetGpioPadDescriptorAndStatePairs pairs);

protected:
    void run(Services::TargetControllerService& targetControllerService) override;

private:
    const Targets::TargetPadDescriptors& padDescriptors;
};
