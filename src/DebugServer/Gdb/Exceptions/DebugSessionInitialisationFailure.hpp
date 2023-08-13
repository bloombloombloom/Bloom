#pragma once

#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::Exceptions
{
    /**
     * The GDB server may fail to prepare for a debug session, if an internal error occurs. One circumstance where
     * this can happen is when the TargetController is not able to service the debug session for whatever reason.
     *
     * See GdbRspDebugServer::serve() for handling code.
     */
    class DebugSessionInitialisationFailure: public ::Exceptions::Exception
    {
    public:
        explicit DebugSessionInitialisationFailure(const std::string& message)
            : ::Exceptions::Exception(message)
        {}

        explicit DebugSessionInitialisationFailure(const char* message)
            : ::Exceptions::Exception(message)
        {}

        explicit DebugSessionInitialisationFailure() = default;
    };
}
