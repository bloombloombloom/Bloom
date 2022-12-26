#include "ReadStackPointer.hpp"

namespace Bloom
{
    using Services::TargetControllerService;

    void ReadStackPointer::run(TargetControllerService& targetControllerService) {
        emit this->stackPointerRead(targetControllerService.getStackPointer());
    }
}
