#include "SetTargetPinState.hpp"

namespace Bloom
{
    using Services::TargetControllerService;

    void SetTargetPinState::run(TargetControllerService& targetControllerService) {
        targetControllerService.setPinState(this->pinDescriptor, this->pinState);
    }
}
