#pragma once

#include "Exception.hpp"

namespace Bloom::Exceptions
{
    class DeviceCommunicationFailure: public Exception
    {
    public:
        explicit DeviceCommunicationFailure(const std::string& message): Exception(message) {
            this->message = message;
        }

        explicit DeviceCommunicationFailure(const char* message): Exception(message) {
            this->message = std::string(message);
        }
    };
}
