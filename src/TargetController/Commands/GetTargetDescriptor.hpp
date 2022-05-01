#pragma once

#include "Command.hpp"

#include "src/TargetController/Responses/TargetDescriptor.hpp"

namespace Bloom::TargetController::Commands
{
    class GetTargetDescriptor: public Command
    {
    public:
        using SuccessResponseType = Responses::TargetDescriptor;

        static constexpr CommandType type = CommandType::GET_TARGET_DESCRIPTOR;
        static inline const std::string name = "GetTargetDescriptor";

        [[nodiscard]] CommandType getType() const override {
            return GetTargetDescriptor::type;
        }
    };
}
