#pragma once

#include "src/Exceptions/Exception.hpp"

namespace Exceptions
{
    class DeviceNotFound: public Exception
    {
    public:
        explicit DeviceNotFound(const std::string& message): Exception(message) {
            this->message = message;
        }

        explicit DeviceNotFound(const char* message): Exception(message) {
            this->message = std::string(message);
        }
    };
}
