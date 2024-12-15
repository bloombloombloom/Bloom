#pragma once

#include "Command.hpp"
#include "src/TargetController/Responses/TargetPassthroughResponse.hpp"

#include "src/Targets/PassthroughCommand.hpp"

namespace TargetController::Commands
{
    class InvokeTargetPassthroughCommand: public Command
    {
    public:
        using SuccessResponseType = Responses::TargetPassthroughResponse;
        static constexpr CommandType type = CommandType::INVOKE_TARGET_PASSTHROUGH_COMMAND;
        static const inline std::string name = "InvokeTargetPassthroughCommand";

        Targets::PassthroughCommand command;

        explicit InvokeTargetPassthroughCommand(Targets::PassthroughCommand&& command)
            : command(std::move(command))
        {};

        [[nodiscard]] CommandType getType() const override {
            return InvokeTargetPassthroughCommand::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }
    };
}
