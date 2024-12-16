#pragma once

#include "Command.hpp"
#include "src/TargetController/Responses/TargetPassthroughHelpText.hpp"

namespace TargetController::Commands
{
    class GetTargetPassthroughHelpText: public Command
    {
    public:
        using SuccessResponseType = Responses::TargetPassthroughHelpText;
        static constexpr CommandType type = CommandType::GET_TARGET_PASSTHROUGH_HELP_TEXT;
        static const inline std::string name = "GetTargetPassthroughHelpText";

        [[nodiscard]] CommandType getType() const override {
            return GetTargetPassthroughHelpText::type;
        }
    };
}
