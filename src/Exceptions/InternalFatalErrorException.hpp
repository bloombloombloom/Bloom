#pragma once

#include "Exception.hpp"
#include "src/Services/PathService.hpp"

namespace Exceptions
{
    class InternalFatalErrorException: public Exception
    {
    public:
        explicit InternalFatalErrorException(const std::string& message)
            : Exception(
                "Internal fatal error - " + message + " - please report this via "
                    + Services::PathService::homeDomainName() + "/report-issue"
            )
        {}

        explicit InternalFatalErrorException(const char* message)
            : InternalFatalErrorException(std::string(message))
        {}
    };
}
