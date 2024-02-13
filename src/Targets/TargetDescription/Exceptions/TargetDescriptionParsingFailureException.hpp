#pragma once

#include "src/Exceptions/InternalFatalErrorException.hpp"

namespace Targets::TargetDescription::Exceptions
{
    class TargetDescriptionParsingFailureException: public ::Exceptions::InternalFatalErrorException
    {
    public:
        explicit TargetDescriptionParsingFailureException(const std::string& message)
            : ::Exceptions::InternalFatalErrorException("Failed to parse target description file - " + message)
        {}
    };
}
