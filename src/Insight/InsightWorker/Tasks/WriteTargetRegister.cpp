#include "WriteTargetRegister.hpp"

using namespace Bloom;

void WriteTargetRegister::run(TargetControllerConsole& targetControllerConsole) {
    targetControllerConsole.writeRegisters({this->targetRegister});
}
