#include "RefreshTargetPinStates.hpp"

namespace Bloom
{
    void RefreshTargetPinStates::run(TargetControllerConsole& targetControllerConsole) {
        emit this->targetPinStatesRetrieved(targetControllerConsole.getPinStates(this->variantId));
    }
}
