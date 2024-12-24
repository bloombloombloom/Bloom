#pragma once

#include "InsightWorkerTask.hpp"
#include "src/Targets/TargetPadDescriptor.hpp"
#include "src/Targets/TargetGpioPadState.hpp"

class SetTargetGpioPadState: public InsightWorkerTask
{
    Q_OBJECT

public:
    SetTargetGpioPadState(const Targets::TargetPadDescriptor& padDescriptor, const Targets::TargetGpioPadState& state);
    [[nodiscard]] QString brief() const override;
    [[nodiscard]] TaskGroups taskGroups() const override;

protected:
    void run(Services::TargetControllerService& targetControllerService) override;

private:
    const Targets::TargetPadDescriptor& padDescriptor;
    Targets::TargetGpioPadState state;
};
