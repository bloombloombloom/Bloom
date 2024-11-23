#pragma once

#include "Exception.hpp"

namespace Exceptions
{
    class FatalErrorException: public Exception
    {
    public:
        explicit FatalErrorException(const std::string& message)
            : Exception(message)
        {}
    };
}
