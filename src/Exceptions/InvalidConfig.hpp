#pragma once

#include "FatalErrorException.hpp"

namespace Exceptions
{
    class InvalidConfig: public FatalErrorException
    {
    public:
        explicit InvalidConfig(const std::string& message)
            : FatalErrorException(message)
        {}
    };
}
