#include "ReadStackPointer.hpp"

namespace Bloom
{
    void ReadStackPointer::run(TargetControllerConsole& targetControllerConsole) {
        emit this->stackPointerRead(targetControllerConsole.getStackPointer());
    }
}
