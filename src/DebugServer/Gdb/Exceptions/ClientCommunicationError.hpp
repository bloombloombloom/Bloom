#pragma once

#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::Exceptions
{
    /**
     * In the event that communication between the GDB RSP client and Bloom fails, a ClientCommunicationFailure
     * exception should be thrown. The GDB debug server handles this by severing the connection.
     *
     * See GdbRspDebugServer::serve() for handling code.
     */
    class ClientCommunicationError: public ::Exceptions::Exception
    {
    public:
        explicit ClientCommunicationError(const std::string& message)
            : ::Exceptions::Exception(message)
        {}

        explicit ClientCommunicationError(const char* message)
            : ::Exceptions::Exception(message)
        {}

        explicit ClientCommunicationError() = default;
    };
}
