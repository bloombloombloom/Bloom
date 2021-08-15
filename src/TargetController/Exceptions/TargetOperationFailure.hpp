#pragma once

#include "src/Exceptions/Exception.hpp"

namespace Bloom::Exceptions
{
    class TargetOperationFailure: public Exception
    {
    public:
        explicit TargetOperationFailure(const std::string& message): Exception(message) {
            this->message = message;
        }

        explicit TargetOperationFailure(const char* message): Exception(message) {
            this->message = std::string(message);
        }
    };
}
