#include "ReadStackPointer.hpp"

using Services::TargetControllerService;

QString ReadStackPointer::brief() const {
    return "Reading stack pointer";
}

TaskGroups ReadStackPointer::taskGroups() const {
    return {
        TaskGroup::USES_TARGET_CONTROLLER,
    };
}

void ReadStackPointer::run(TargetControllerService& targetControllerService) {
    emit this->stackPointerRead(targetControllerService.getStackPointer());
}
