#include "ReadTargetRegisters.hpp"

namespace Bloom
{
    using TargetController::TargetControllerConsole;

    void ReadTargetRegisters::run(TargetControllerConsole& targetControllerConsole) {
        emit this->targetRegistersRead(targetControllerConsole.readRegisters(this->descriptors));
    }
}
