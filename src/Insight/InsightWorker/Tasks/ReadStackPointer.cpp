#include "ReadStackPointer.hpp"

using Services::TargetControllerService;

void ReadStackPointer::run(TargetControllerService& targetControllerService) {
    emit this->stackPointerRead(targetControllerService.getStackPointer());
}
