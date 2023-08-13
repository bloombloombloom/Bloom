#include "GetTargetDescriptor.hpp"

using Services::TargetControllerService;

void GetTargetDescriptor::run(TargetControllerService& targetControllerService) {
    emit this->targetDescriptor(targetControllerService.getTargetDescriptor());
}
