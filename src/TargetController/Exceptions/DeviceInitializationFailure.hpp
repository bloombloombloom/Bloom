#pragma once

#include "DeviceFailure.hpp"

namespace Exceptions
{
    class DeviceInitializationFailure: public DeviceFailure
    {
    public:
        explicit DeviceInitializationFailure(const std::string& message)
            : DeviceFailure(message)
        {}
    };
}
