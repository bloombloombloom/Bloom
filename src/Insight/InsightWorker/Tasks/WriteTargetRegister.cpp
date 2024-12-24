#include "WriteTargetRegister.hpp"

using Services::TargetControllerService;

WriteTargetRegister::WriteTargetRegister(const Targets::TargetRegisterDescriptorAndValuePair& registerPair)
    : registerPair(registerPair)
{}

QString WriteTargetRegister::brief() const {
    return "Writing to target register \"" + QString::fromStdString(this->registerPair.first.name) + "\"";
}

TaskGroups WriteTargetRegister::taskGroups() const {
    return {
        TaskGroup::USES_TARGET_CONTROLLER,
    };
}

void WriteTargetRegister::run(TargetControllerService& targetControllerService) {
    targetControllerService.writeRegisters({this->registerPair});
}
