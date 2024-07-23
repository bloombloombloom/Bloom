#pragma once

#include "Exception.hpp"

namespace Exceptions
{
    class TargetControllerStartupFailure: public Exception
    {
    public:
        explicit TargetControllerStartupFailure(const std::string& message)
            : Exception(message)
        {}
    };
}
