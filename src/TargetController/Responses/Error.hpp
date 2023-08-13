#pragma once

#include "Response.hpp"

namespace TargetController::Responses
{
    class Error: public Response
    {
    public:
        static constexpr ResponseType type = ResponseType::ERROR;

        std::string errorMessage;

        explicit Error(const std::string& errorMessage)
            : errorMessage(errorMessage)
        {}

        [[nodiscard]] ResponseType getType() const override {
            return Error::type;
        }
    };
}
