#include "ReadProgramCounter.hpp"

namespace Bloom
{
    using TargetController::TargetControllerConsole;

    void ReadProgramCounter::run(TargetControllerConsole& targetControllerConsole) {
        emit this->programCounterRead(targetControllerConsole.getProgramCounter());
    }
}
