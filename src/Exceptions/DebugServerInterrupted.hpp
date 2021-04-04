#pragma once

#include "Exception.hpp"
namespace Bloom::Exceptions
{
    class DebugServerInterrupted: public Exception
    {
    public:
        explicit DebugServerInterrupted() = default;

    };
}
