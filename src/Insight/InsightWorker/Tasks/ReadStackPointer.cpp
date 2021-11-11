#include "ReadStackPointer.hpp"

using namespace Bloom;

void ReadStackPointer::run(TargetControllerConsole& targetControllerConsole) {
    emit this->stackPointerRead(targetControllerConsole.getStackPointer());
}
