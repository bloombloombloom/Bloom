#pragma once

#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::Exceptions
{
    /**
     * In the event that the GDB debug server determines that the connected client cannot be served,
     * the ClientNotSupported exception should be thrown.
     *
     * See GdbRspDebugServer::serve() for handling code.
     */
    class ClientNotSupported: public ::Exceptions::Exception
    {
    public:
        explicit ClientNotSupported(const std::string& message)
            : ::Exceptions::Exception(message)
        {}

        explicit ClientNotSupported(const char* message)
            : ::Exceptions::Exception(message)
        {}

        explicit ClientNotSupported() = default;
    };
}
