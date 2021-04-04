#pragma once

#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServers::Gdb::Exceptions
{
    using namespace Bloom::Exceptions;

    /**
     * In the event that the GDB debug server determines that the connected client cannot be served,
     * the ClientNotSupported exception should be thrown.
     *
     * See GdbRspDebugServer::serve() for handling code.
     */
    class ClientNotSupported: public Exception
    {
    public:
        explicit ClientNotSupported(const std::string& message) : Exception(message) {
            this->message = message;
        }

        explicit ClientNotSupported(const char* message) : Exception(message) {
            this->message = std::string(message);
        }

        explicit ClientNotSupported() = default;
    };
}
