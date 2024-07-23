#pragma once

#include "src/Exceptions/Exception.hpp"

namespace Exceptions
{
    class DeviceFailure: public Exception
    {
    public:
        explicit DeviceFailure(const std::string& message)
            : Exception(message)
        {}
    };
}
