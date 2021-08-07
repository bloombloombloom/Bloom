#include "AvrGdbRsp.hpp"

#include "src/Exceptions/Exception.hpp"

using namespace Bloom::DebugServers::Gdb;
using namespace Bloom::Exceptions;

using Bloom::Targets::TargetRegisterDescriptor;
using Bloom::Targets::TargetRegisterType;

void AvrGdbRsp::loadRegisterNumberToDescriptorMapping() {
    auto& registerDescriptorsByType = this->targetDescriptor.registerDescriptorsByType;
    if (!registerDescriptorsByType.contains(TargetRegisterType::STATUS_REGISTER)) {
        throw Exception("Missing status register descriptor");
    }

    if (!registerDescriptorsByType.contains(TargetRegisterType::STACK_POINTER)) {
        throw Exception("Missing stack pointer register descriptor");
    }

    if (!registerDescriptorsByType.contains(TargetRegisterType::PROGRAM_COUNTER)) {
        throw Exception("Missing program counter register descriptor");
    }

    if (!registerDescriptorsByType.contains(TargetRegisterType::GENERAL_PURPOSE_REGISTER)
        || registerDescriptorsByType.at(TargetRegisterType::GENERAL_PURPOSE_REGISTER).size() != 32) {
        throw Exception("Unexpected general purpose register count");
    }

    auto& gpRegisterDescriptors = registerDescriptorsByType.at(TargetRegisterType::GENERAL_PURPOSE_REGISTER);
    for (const auto& gpRegisterDescriptor : gpRegisterDescriptors) {
        this->registerNumberToDescriptorMapping.insert(std::pair(
            static_cast<GdbRegisterNumber>(gpRegisterDescriptor.id.value()),
            gpRegisterDescriptor
        ));
    }

    this->registerNumberToDescriptorMapping.insert(std::pair(
        static_cast<GdbRegisterNumber>(32),
        registerDescriptorsByType.at(TargetRegisterType::STATUS_REGISTER).front()
    ));

    this->registerNumberToDescriptorMapping.insert(std::pair(
        static_cast<GdbRegisterNumber>(33),
        registerDescriptorsByType.at(TargetRegisterType::STACK_POINTER).front()
    ));

    this->registerNumberToDescriptorMapping.insert(std::pair(
        static_cast<GdbRegisterNumber>(34),
        registerDescriptorsByType.at(TargetRegisterType::PROGRAM_COUNTER).front()
    ));
}

void AvrGdbRsp::init() {
    this->loadRegisterNumberToDescriptorMapping();

    GdbRspDebugServer::init();
}
