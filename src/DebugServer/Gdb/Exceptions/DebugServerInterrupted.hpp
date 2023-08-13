#pragma once

#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::Exceptions
{
    /**
     * The GDB debug server spends most of its time in a blocking state, waiting for a new connection or for some data
     * from the connected GDB client. The server implementation allows for interruptions to blocking IO calls.
     *
     * When an interrupt occurs, this exception is thrown and handled appropriately.
     *
     * For more on how the GDB server implementation allows for interruptions, see the "Servicing events" section in
     * src/DebugServer/README.md.
     */
    class DebugServerInterrupted: public ::Exceptions::Exception
    {
    public:
        explicit DebugServerInterrupted() = default;
    };
}
