#pragma once

#include "src/Exceptions/FatalErrorException.hpp"

namespace Exceptions
{
    class DeviceFailure: public FatalErrorException
    {
    public:
        explicit DeviceFailure(const std::string& message)
            : FatalErrorException(message)
        {}
    };
}
