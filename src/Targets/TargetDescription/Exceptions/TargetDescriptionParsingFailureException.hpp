#pragma once

#include "src/Exceptions/Exception.hpp"

namespace Exceptions
{
    class TargetDescriptionParsingFailureException: public Exception
    {
    public:
        explicit TargetDescriptionParsingFailureException(const std::string& message)
        : Exception(message) {
            this->message = "Failed to parse target description file - " + message;
        }

        explicit TargetDescriptionParsingFailureException(const char* message)
        : Exception(message) {
            this->message = "Failed to parse target description file - " + std::string(message);
        }
    };
}
