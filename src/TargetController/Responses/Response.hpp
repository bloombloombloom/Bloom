#pragma once

#include "ResponseTypes.hpp"

namespace TargetController::Responses
{
    class Response
    {
    public:
        static constexpr ResponseType type = ResponseType::GENERIC;

        Response() = default;
        virtual ~Response() = default;

        Response(const Response& other) = default;
        Response(Response&& other) = default;

        Response& operator = (const Response& other) = default;
        Response& operator = (Response&& other) = default;

        [[nodiscard]] virtual ResponseType getType() const {
            return Response::type;
        }
    };
}
