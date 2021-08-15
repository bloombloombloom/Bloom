#pragma once

#include "src/Exceptions/Exception.hpp"

namespace Bloom::Exceptions
{
    class DeviceFailure: public Exception
    {
    public:
        explicit DeviceFailure(const std::string& message): Exception(message) {
            this->message = message;
        }

        explicit DeviceFailure(const char* message): Exception(message) {
            this->message = std::string(message);
        }
    };
}
