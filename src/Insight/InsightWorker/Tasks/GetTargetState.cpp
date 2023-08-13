#include "GetTargetState.hpp"

using Services::TargetControllerService;

void GetTargetState::run(TargetControllerService& targetControllerService) {
    emit this->targetState(targetControllerService.getTargetState());
}
