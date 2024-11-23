#pragma once

#include "src/Exceptions/Exception.hpp"

namespace Exceptions
{
    class DeviceNotFound: public FatalErrorException
    {
    public:
        explicit DeviceNotFound(const std::string& message)
            : FatalErrorException(message)
        {}
    };
}
