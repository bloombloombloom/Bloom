#pragma once

#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServers::Gdb::Exceptions
{
    using namespace Bloom::Exceptions;

    /**
     * In the event that communication between the GDB RSP client and Bloom fails, a ClientCommunicationFailure
     * exception should be thrown. The GDB debug server handles this by severing the connection.
     *
     * See GdbRspDebugServer::serve() for handling code.
     */
    class ClientCommunicationError: public Exception
    {
    public:
        explicit ClientCommunicationError(const std::string& message): Exception(message) {
            this->message = message;
        }

        explicit ClientCommunicationError(const char* message): Exception(message) {
            this->message = std::string(message);
        }

        explicit ClientCommunicationError() = default;
    };
}
