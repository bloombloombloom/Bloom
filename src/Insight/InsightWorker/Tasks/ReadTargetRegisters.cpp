#include "ReadTargetRegisters.hpp"

using Services::TargetControllerService;

void ReadTargetRegisters::run(TargetControllerService& targetControllerService) {
    emit this->targetRegistersRead(targetControllerService.readRegisters(this->descriptorIds));
}
