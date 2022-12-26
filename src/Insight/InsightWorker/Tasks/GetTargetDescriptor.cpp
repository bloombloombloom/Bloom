#include "GetTargetDescriptor.hpp"

namespace Bloom
{
    using Services::TargetControllerService;

    void GetTargetDescriptor::run(TargetControllerService& targetControllerService) {
        emit this->targetDescriptor(targetControllerService.getTargetDescriptor());
    }
}
