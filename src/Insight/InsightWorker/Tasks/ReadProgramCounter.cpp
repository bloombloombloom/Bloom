#include "ReadProgramCounter.hpp"

namespace Bloom
{
    using Services::TargetControllerService;

    void ReadProgramCounter::run(TargetControllerService& targetControllerService) {
        emit this->programCounterRead(targetControllerService.getProgramCounter());
    }
}
