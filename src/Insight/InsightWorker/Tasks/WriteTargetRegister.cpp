#include "WriteTargetRegister.hpp"

namespace Bloom
{
    using Services::TargetControllerService;

    void WriteTargetRegister::run(TargetControllerService& targetControllerService) {
        targetControllerService.writeRegisters({this->targetRegister});
    }
}
