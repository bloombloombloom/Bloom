#include "ReadTargetRegisters.hpp"

namespace Bloom
{
    using Services::TargetControllerService;

    void ReadTargetRegisters::run(TargetControllerService& targetControllerService) {
        emit this->targetRegistersRead(targetControllerService.readRegisters(this->descriptors));
    }
}
