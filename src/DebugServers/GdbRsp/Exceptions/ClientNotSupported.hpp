#pragma once

#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServer::Gdb::Exceptions
{
    /**
     * In the event that the GDB debug server determines that the connected client cannot be served,
     * the ClientNotSupported exception should be thrown.
     *
     * See GdbRspDebugServer::serve() for handling code.
     */
    class ClientNotSupported: public Bloom::Exceptions::Exception
    {
    public:
        explicit ClientNotSupported(const std::string& message): Bloom::Exceptions::Exception(message) {
            this->message = message;
        }

        explicit ClientNotSupported(const char* message): Bloom::Exceptions::Exception(message) {
            this->message = std::string(message);
        }

        explicit ClientNotSupported() = default;
    };
}
