#pragma once

#include "Command.hpp"

#include "src/TargetController/Responses/AtomicSessionId.hpp"

namespace Bloom::TargetController::Commands
{
    class EndAtomicSession: public Command
    {
    public:
        static constexpr CommandType type = CommandType::END_ATOMIC_SESSION;
        static const inline std::string name = "EndAtomicSession";

        AtomicSessionIdType sessionId;

        explicit EndAtomicSession(AtomicSessionIdType sessionId)
            : sessionId(sessionId)
        {}

        [[nodiscard]] CommandType getType() const override {
            return EndAtomicSession::type;
        }
    };
}
