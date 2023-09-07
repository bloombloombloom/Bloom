#pragma once

#include "src/Exceptions/Exception.hpp"

namespace Exceptions
{
    class TargetOperationFailure: public Exception
    {
    public:
        explicit TargetOperationFailure(const std::string& message)
            : Exception(message)
        {}

        explicit TargetOperationFailure(const char* message)
            : Exception(message)
        {}
    };
}
