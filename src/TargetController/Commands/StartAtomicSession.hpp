#pragma once

#include "Command.hpp"

#include "src/TargetController/Responses/AtomicSessionId.hpp"

namespace TargetController::Commands
{
    class StartAtomicSession: public Command
    {
    public:
        using SuccessResponseType = Responses::AtomicSessionId;

        static constexpr CommandType type = CommandType::START_ATOMIC_SESSION;
        static const inline std::string name = "StartAtomicSession";

        [[nodiscard]] CommandType getType() const override {
            return StartAtomicSession::type;
        }
    };
}
