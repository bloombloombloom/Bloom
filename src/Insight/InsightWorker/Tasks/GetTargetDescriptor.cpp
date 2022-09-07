#include "GetTargetDescriptor.hpp"

namespace Bloom
{
    using TargetController::TargetControllerConsole;

    void GetTargetDescriptor::run(TargetControllerConsole& targetControllerConsole) {
        emit this->targetDescriptor(targetControllerConsole.getTargetDescriptor());
    }
}
