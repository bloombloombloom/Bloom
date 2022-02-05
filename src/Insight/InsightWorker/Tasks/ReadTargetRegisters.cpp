#include "ReadTargetRegisters.hpp"

namespace Bloom
{
    void ReadTargetRegisters::run(TargetControllerConsole& targetControllerConsole) {
        emit this->targetRegistersRead(targetControllerConsole.readRegisters(this->descriptors));
    }
}
