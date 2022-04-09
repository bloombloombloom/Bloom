#include "ReadTargetMemory.hpp"

namespace Bloom
{
    using TargetController::TargetControllerConsole;

    void ReadTargetMemory::run(TargetControllerConsole& targetControllerConsole) {
        emit this->targetMemoryRead(
            targetControllerConsole.readMemory(
                this->memoryType,
                this->startAddress,
                this->size,
                this->excludedAddressRanges
            )
        );
    }
}
