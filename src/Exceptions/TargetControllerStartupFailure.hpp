#pragma once

#include "FatalErrorException.hpp"

namespace Exceptions
{
    class TargetControllerStartupFailure: public FatalErrorException
    {
    public:
        explicit TargetControllerStartupFailure(const std::string& message)
            : FatalErrorException(message)
        {}
    };
}
