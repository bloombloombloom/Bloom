#include "RefreshTargetPinStates.hpp"

namespace Bloom
{
    using TargetController::TargetControllerConsole;

    void RefreshTargetPinStates::run(TargetControllerConsole& targetControllerConsole) {
        emit this->targetPinStatesRetrieved(targetControllerConsole.getPinStates(this->variantId));
    }
}
