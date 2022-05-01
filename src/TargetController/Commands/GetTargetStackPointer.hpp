#pragma once

#include "Command.hpp"

#include "src/TargetController/Responses/TargetStackPointer.hpp"

namespace Bloom::TargetController::Commands
{
    class GetTargetStackPointer: public Command
    {
    public:
        using SuccessResponseType = Responses::TargetStackPointer;

        static constexpr CommandType type = CommandType::GET_TARGET_STACK_POINTER;
        static inline const std::string name = "GetTargetStackPointer";

        [[nodiscard]] CommandType getType() const override {
            return GetTargetStackPointer::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }
    };
}
