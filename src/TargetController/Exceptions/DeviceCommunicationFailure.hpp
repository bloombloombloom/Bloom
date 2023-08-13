#pragma once

#include "DeviceFailure.hpp"

namespace Exceptions
{
    class DeviceCommunicationFailure: public DeviceFailure
    {
    public:
        explicit DeviceCommunicationFailure(const std::string& message): DeviceFailure(message) {
            this->message = message;
        }

        explicit DeviceCommunicationFailure(const char* message): DeviceFailure(message) {
            this->message = std::string(message);
        }
    };
}
