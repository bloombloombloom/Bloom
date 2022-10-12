#pragma once

#include "Command.hpp"

#include "src/TargetController/Responses/TargetProgramCounter.hpp"

namespace Bloom::TargetController::Commands
{
    class GetTargetProgramCounter: public Command
    {
    public:
        using SuccessResponseType = Responses::TargetProgramCounter;

        static constexpr CommandType type = CommandType::GET_TARGET_PROGRAM_COUNTER;
        static const inline std::string name = "GetTargetProgramCounter";

        [[nodiscard]] CommandType getType() const override {
            return GetTargetProgramCounter::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }
    };
}
