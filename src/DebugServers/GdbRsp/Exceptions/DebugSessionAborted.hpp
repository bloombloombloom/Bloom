#pragma once

#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServers::Gdb::Exceptions
{
    /**
     * The GDB server may abort a debug session with the client, if an internal error occurs. One circumstance where
     * this can happen is when the TargetController is not able to service the debug session for whatever reason.
     *
     * See GdbRspDebugServer::serve() for handling code.
     */
    class DebugSessionAborted: public Bloom::Exceptions::Exception
    {
    public:
        explicit DebugSessionAborted(const std::string& message): Bloom::Exceptions::Exception(message) {
            this->message = message;
        }

        explicit DebugSessionAborted(const char* message): Bloom::Exceptions::Exception(message) {
            this->message = std::string(message);
        }

        explicit DebugSessionAborted() = default;
    };
}
