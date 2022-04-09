#include "SetTargetPinState.hpp"

namespace Bloom
{
    using TargetController::TargetControllerConsole;

    void SetTargetPinState::run(TargetControllerConsole& targetControllerConsole) {
        targetControllerConsole.setPinState(this->pinDescriptor, this->pinState);
    }
}
