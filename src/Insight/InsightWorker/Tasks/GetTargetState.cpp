#include "GetTargetState.hpp"

namespace Bloom
{
    using Services::TargetControllerService;

    void GetTargetState::run(TargetControllerService& targetControllerService) {
        emit this->targetState(targetControllerService.getTargetState());
    }
}
