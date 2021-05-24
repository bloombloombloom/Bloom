#pragma once

#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServers::Gdb::Exceptions
{
    /**
     * In the event that communication between the GDB RSP client and Bloom fails, a ClientCommunicationFailure
     * exception should be thrown. The GDB debug server handles this by severing the connection.
     *
     * See GdbRspDebugServer::serve() for handling code.
     */
    class ClientCommunicationError: public Bloom::Exceptions::Exception
    {
    public:
        explicit ClientCommunicationError(const std::string& message): Bloom::Exceptions::Exception(message) {
            this->message = message;
        }

        explicit ClientCommunicationError(const char* message): Bloom::Exceptions::Exception(message) {
            this->message = std::string(message);
        }

        explicit ClientCommunicationError() = default;
    };
}
