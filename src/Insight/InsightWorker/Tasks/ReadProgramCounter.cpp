#include "ReadProgramCounter.hpp"

using Services::TargetControllerService;

void ReadProgramCounter::run(TargetControllerService& targetControllerService) {
    emit this->programCounterRead(targetControllerService.getProgramCounter());
}
