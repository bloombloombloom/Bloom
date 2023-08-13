#pragma once

#include <cstdint>

#include "Response.hpp"

#include "src/TargetController/AtomicSession.hpp"

namespace TargetController::Responses
{
    class AtomicSessionId: public Response
    {
    public:
        static constexpr ResponseType type = ResponseType::ATOMIC_SESSION_ID;

        AtomicSessionIdType sessionId;

        explicit AtomicSessionId(AtomicSessionIdType sessionId)
            : sessionId(sessionId)
        {}

        [[nodiscard]] ResponseType getType() const override {
            return AtomicSessionId::type;
        }
    };
}
