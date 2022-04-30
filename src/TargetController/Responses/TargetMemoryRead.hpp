#pragma once

#include "Response.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace Bloom::TargetController::Responses
{
    class TargetMemoryRead: public Response
    {
    public:
        static constexpr ResponseType type = ResponseType::TARGET_MEMORY_READ;

        Targets::TargetMemoryBuffer data;

        explicit TargetMemoryRead(const Targets::TargetMemoryBuffer& data)
            : data(data)
        {}

        [[nodiscard]] ResponseType getType() const override {
            return TargetMemoryRead::type;
        }
    };
}
