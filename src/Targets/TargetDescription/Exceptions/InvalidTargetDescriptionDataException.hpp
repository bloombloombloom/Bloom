#pragma once

#include "src/Exceptions/InternalFatalErrorException.hpp"

namespace Targets::TargetDescription::Exceptions
{
    class InvalidTargetDescriptionDataException: public ::Exceptions::InternalFatalErrorException
    {
    public:
        explicit InvalidTargetDescriptionDataException(const std::string& message)
            : ::Exceptions::InternalFatalErrorException("Missing/invalid data in TDF - " + message)
        {}
    };
}
