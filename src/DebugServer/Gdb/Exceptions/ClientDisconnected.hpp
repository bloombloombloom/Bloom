#pragma once

#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::Exceptions
{
    /**
     * When a GDB RSP client unexpectedly drops the connection in the middle of an IO operation, a ClientDisconnected
     * exception should be thrown. The GDB debug server handles this by clearing the connection and waiting for a new
     * one.
     *
     * See GdbRspDebugServer::serve() for handling code.
     */
    class ClientDisconnected: public ::Exceptions::Exception
    {
    public:
        explicit ClientDisconnected() = default;
    };
}
