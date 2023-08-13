#include "RefreshTargetPinStates.hpp"

using Services::TargetControllerService;

void RefreshTargetPinStates::run(TargetControllerService& targetControllerService) {
    emit this->targetPinStatesRetrieved(targetControllerService.getPinStates(this->variantId));
}
