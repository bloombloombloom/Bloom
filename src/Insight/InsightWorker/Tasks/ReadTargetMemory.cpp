#include "ReadTargetMemory.hpp"

using namespace Bloom;

void ReadTargetMemory::run(TargetControllerConsole& targetControllerConsole) {
    emit this->targetMemoryRead(
        targetControllerConsole.readMemory(this->memoryType, this->startAddress, this->size)
    );
}
