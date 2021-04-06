#pragma once

#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServers::Gdb::Exceptions
{
    using namespace Bloom::Exceptions;

    /**
     * When a GDB RSP client unexpectedly drops the connection in the middle of an IO operation, a ClientDisconnected
     * exception should be thrown. The GDB debug server handles this by clearing the connection and waiting for a new
     * one.
     *
     * See GdbRspDebugServer::serve() for handling code.
     */
    class ClientDisconnected: public Exception
    {
    public:
        explicit ClientDisconnected(const std::string& message): Exception(message) {
            this->message = message;
        }

        explicit ClientDisconnected(const char* message): Exception(message) {
            this->message = std::string(message);
        }

        explicit ClientDisconnected() = default;
    };
}
