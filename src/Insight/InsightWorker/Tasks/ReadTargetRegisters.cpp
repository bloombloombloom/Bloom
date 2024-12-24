#include "ReadTargetRegisters.hpp"

#include <QLocale>

using Services::TargetControllerService;

ReadTargetRegisters::ReadTargetRegisters(const Targets::TargetRegisterDescriptors& descriptors)
    : descriptors(descriptors)
{}

QString ReadTargetRegisters::brief() const {
    return this->descriptors.size() == 1
        ? "Reading \"" + QString::fromStdString(this->descriptors.front()->name) + "\" target register"
        : "Reading " + QLocale{QLocale::English}.toString(this->descriptors.size()) + " target registers";
}

TaskGroups ReadTargetRegisters::taskGroups() const {
    return {
        TaskGroup::USES_TARGET_CONTROLLER,
    };
}

void ReadTargetRegisters::run(TargetControllerService& targetControllerService) {
    emit this->targetRegistersRead(targetControllerService.readRegisters(this->descriptors));
}
