#pragma once

#include "Exception.hpp"

namespace Exceptions
{
    class InvalidConfig: public Exception
    {
    public:
        explicit InvalidConfig(const std::string& message): Exception(message) {
            this->message = message;
        }

        explicit InvalidConfig(const char* message): Exception(message) {
            this->message = std::string(message);
        }
    };
}
