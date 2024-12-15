#pragma once

#include <optional>

#include "Response.hpp"

#include "src/Targets/PassthroughResponse.hpp"

namespace TargetController::Responses
{
    class TargetPassthroughResponse: public Response
    {
    public:
        static constexpr ResponseType type = ResponseType::TARGET_PASSTHROUGH_RESPONSE;

        std::optional<Targets::PassthroughResponse> response;

        explicit TargetPassthroughResponse(std::optional<Targets::PassthroughResponse>&& response)
            : response(std::move(response))
        {}

        [[nodiscard]] ResponseType getType() const override {
            return TargetPassthroughResponse::type;
        }
    };
}
