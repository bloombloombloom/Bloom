#include "WriteTargetRegister.hpp"

namespace Bloom
{
    using TargetController::TargetControllerConsole;

    void WriteTargetRegister::run(TargetControllerConsole& targetControllerConsole) {
        targetControllerConsole.writeRegisters({this->targetRegister});
    }
}
