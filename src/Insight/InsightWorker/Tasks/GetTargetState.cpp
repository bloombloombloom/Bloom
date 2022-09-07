#include "GetTargetState.hpp"

namespace Bloom
{
    using TargetController::TargetControllerConsole;

    void GetTargetState::run(TargetControllerConsole& targetControllerConsole) {
        emit this->targetState(targetControllerConsole.getTargetState());
    }
}
