#pragma once

#include "src/TargetController/Exceptions/TargetOperationFailure.hpp"

namespace Exceptions
{
    class IllegalMemoryAccess: public TargetOperationFailure
    {
    public:
        explicit IllegalMemoryAccess()
            : TargetOperationFailure("Illegal memory access")
        {}

        explicit IllegalMemoryAccess(const std::string& message)
            : TargetOperationFailure(message)
        {}
    };
}
