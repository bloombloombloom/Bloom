#pragma once

#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::Exceptions
{
    /**
     * The GDB debug server spends most of its time in a blocking state, waiting for a new connection or for some data
     * from the connected GDB client. The server implementation allows for interruptions to blocking IO calls.
     *
     * When an interrupt occurs, this exception is thrown and handled appropriately.
     */
    class DebugServerInterrupted: public ::Exceptions::Exception
    {
    public:
        DebugServerInterrupted() = default;
    };
}
