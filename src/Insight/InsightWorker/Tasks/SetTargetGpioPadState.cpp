#include "SetTargetGpioPadState.hpp"

using Services::TargetControllerService;

SetTargetGpioPadState::SetTargetGpioPadState(
    const Targets::TargetPadDescriptor& padDescriptor,
    const Targets::TargetGpioPadState& state
)
    : padDescriptor(padDescriptor)
    , state(state)
{}

QString SetTargetGpioPadState::brief() const {
    return "Updating target pin state";
}

TaskGroups SetTargetGpioPadState::taskGroups() const {
    return {
        TaskGroup::USES_TARGET_CONTROLLER,
    };
}

void SetTargetGpioPadState::run(TargetControllerService& targetControllerService) {
    targetControllerService.setGpioPadState(this->padDescriptor, this->state);
}
