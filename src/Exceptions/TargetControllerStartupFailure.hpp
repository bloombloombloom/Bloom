#pragma once

#include "Exception.hpp"
namespace Bloom::Exceptions
{
    class TargetControllerStartupFailure: public Exception
    {
    public:
        explicit TargetControllerStartupFailure(const std::string& message) : Exception(message) {
            this->message = message;
        }

        explicit TargetControllerStartupFailure(const char* message) : Exception(message) {
            this->message = std::string(message);
        }
    };
}
