#pragma once

#include "Command.hpp"
#include "src/TargetController/Responses/TargetRegistersRead.hpp"

#include "src/Targets/TargetRegister.hpp"

namespace Bloom::TargetController::Commands
{
    class ReadTargetRegisters: public Command
    {
    public:
        using SuccessResponseType = Responses::TargetRegistersRead;

        static constexpr CommandType type = CommandType::READ_TARGET_REGISTERS;
        static inline const std::string name = "ReadTargetRegisters";

        Targets::TargetRegisterDescriptors descriptors;

        explicit ReadTargetRegisters(const Targets::TargetRegisterDescriptors& descriptors)
            : descriptors(descriptors)
        {};

        [[nodiscard]] CommandType getType() const override {
            return ReadTargetRegisters::type;
        }
    };
}
