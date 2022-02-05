#include "WriteTargetRegister.hpp"

namespace Bloom
{
    void WriteTargetRegister::run(TargetControllerConsole& targetControllerConsole) {
        targetControllerConsole.writeRegisters({this->targetRegister});
    }
}
