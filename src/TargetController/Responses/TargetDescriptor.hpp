#pragma once

#include <cstdint>

#include "Response.hpp"

#include "src/Targets/TargetDescriptor.hpp"

namespace TargetController::Responses
{
    class TargetDescriptor: public Response
    {
    public:
        static constexpr ResponseType type = ResponseType::TARGET_DESCRIPTOR;

        const Targets::TargetDescriptor& targetDescriptor;

        explicit TargetDescriptor(const Targets::TargetDescriptor& targetDescriptor)
            : targetDescriptor(targetDescriptor)
        {}

        [[nodiscard]] ResponseType getType() const override {
            return TargetDescriptor::type;
        }
    };
}
