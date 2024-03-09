#pragma once

#include "Command.hpp"
#include "src/TargetController/Responses/TargetRegistersRead.hpp"

#include "src/Targets/TargetRegisterDescriptor.hpp"

namespace TargetController::Commands
{
    class ReadTargetRegisters: public Command
    {
    public:
        using SuccessResponseType = Responses::TargetRegistersRead;

        static constexpr CommandType type = CommandType::READ_TARGET_REGISTERS;
        static const inline std::string name = "ReadTargetRegisters";

        std::set<Targets::TargetRegisterDescriptorId> descriptorIds;

        explicit ReadTargetRegisters(const std::set<Targets::TargetRegisterDescriptorId>& descriptorIds)
            : descriptorIds(descriptorIds)
        {};

        [[nodiscard]] CommandType getType() const override {
            return ReadTargetRegisters::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }
    };
}
