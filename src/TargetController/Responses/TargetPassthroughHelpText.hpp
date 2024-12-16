#pragma once

#include <string>

#include "Response.hpp"

namespace TargetController::Responses
{
    class TargetPassthroughHelpText: public Response
    {
    public:
        static constexpr ResponseType type = ResponseType::TARGET_PASSTHROUGH_RESPONSE;

        std::string text;

        explicit TargetPassthroughHelpText(std::string&& text)
            : text(std::move(text))
        {}

        [[nodiscard]] ResponseType getType() const override {
            return TargetPassthroughHelpText::type;
        }
    };
}
