#include "SetTargetPinState.hpp"

namespace Bloom
{
    void SetTargetPinState::run(TargetControllerConsole& targetControllerConsole) {
        targetControllerConsole.setPinState(this->pinDescriptor, this->pinState);
    }
}
