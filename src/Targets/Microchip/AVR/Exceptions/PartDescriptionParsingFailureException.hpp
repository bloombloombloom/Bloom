#pragma once

#include "src/Exceptions/TargetControllerStartupFailure.hpp"

namespace Bloom::Exceptions
{
    class PartDescriptionParsingFailureException: public Exception
    {
    public:
        explicit PartDescriptionParsingFailureException(const std::string& message)
        : Exception(message) {
            this->message = "Failed to parse AVR part description file - " + message;
        }

        explicit PartDescriptionParsingFailureException(const char* message)
        : Exception(message) {
            this->message = "Failed to parse AVR part description file - " + std::string(message);
        }
    };
}
