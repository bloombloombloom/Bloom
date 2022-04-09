#include "ReadStackPointer.hpp"

namespace Bloom
{
    using TargetController::TargetControllerConsole;

    void ReadStackPointer::run(TargetControllerConsole& targetControllerConsole) {
        emit this->stackPointerRead(targetControllerConsole.getStackPointer());
    }
}
