#include "WriteTargetRegister.hpp"

using Services::TargetControllerService;

void WriteTargetRegister::run(TargetControllerService& targetControllerService) {
    targetControllerService.writeRegisters({this->targetRegister});
}
