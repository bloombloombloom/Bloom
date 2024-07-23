#pragma once

#include "Exception.hpp"

namespace Exceptions
{
    class InvalidConfig: public Exception
    {
    public:
        explicit InvalidConfig(const std::string& message)
            : Exception(message)
        {}
    };
}
