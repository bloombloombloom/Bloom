#pragma once

#include "src/TargetController/Exceptions/TargetOperationFailure.hpp"

namespace Exceptions
{
    class DebugWirePhysicalInterfaceError: public TargetOperationFailure
    {
    public:
        explicit DebugWirePhysicalInterfaceError(const std::string& message)
            : TargetOperationFailure(message)
        {}

        explicit DebugWirePhysicalInterfaceError(const char* message)
            : TargetOperationFailure(message)
        {}
    };
}
