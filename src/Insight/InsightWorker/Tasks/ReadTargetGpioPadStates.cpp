#include "ReadTargetGpioPadStates.hpp"

using Services::TargetControllerService;

ReadTargetGpioPadStates::ReadTargetGpioPadStates(const Targets::TargetPadDescriptors& padDescriptors)
    : padDescriptors(padDescriptors)
{}

QString ReadTargetGpioPadStates::brief() const {
    return "Reading target pin states";
}

TaskGroups ReadTargetGpioPadStates::taskGroups() const {
    return {
        TaskGroup::USES_TARGET_CONTROLLER,
    };
}

void ReadTargetGpioPadStates::run(TargetControllerService& targetControllerService) {
    emit this->targetGpioPadStatesRead(targetControllerService.getGpioPadStates(this->padDescriptors));
}
