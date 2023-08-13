#pragma once

#include "DeviceFailure.hpp"

namespace Exceptions
{
    class DeviceInitializationFailure: public DeviceFailure
    {
    public:
        explicit DeviceInitializationFailure(const std::string& message): DeviceFailure(message) {
            this->message = message;
        }

        explicit DeviceInitializationFailure(const char* message): DeviceFailure(message) {
            this->message = std::string(message);
        }
    };
}
