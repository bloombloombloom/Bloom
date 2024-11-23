#pragma once

#include "FatalErrorException.hpp"
#include "src/Services/PathService.hpp"

namespace Exceptions
{
    class InternalFatalErrorException: public FatalErrorException
    {
    public:
        explicit InternalFatalErrorException(const std::string& message)
            : FatalErrorException(
                "Internal fatal error - " + message + " - please report this via "
                    + Services::PathService::homeDomainName() + "/report-issue"
            )
        {}
    };
}
