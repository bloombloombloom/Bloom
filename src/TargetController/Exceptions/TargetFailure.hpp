#pragma once

#include "src/Exceptions/FatalErrorException.hpp"

namespace Exceptions
{
    class TargetFailure: public FatalErrorException
    {
    public:
        explicit TargetFailure(const std::string& message)
            : FatalErrorException(message)
        {}
    };
}
