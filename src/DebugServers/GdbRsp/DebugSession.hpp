#pragma once

#include <cstdint>

#include "TargetDescriptor.hpp"
#include "Connection.hpp"

namespace Bloom::DebugServers::Gdb
{
    class DebugSession
    {
    public:
        Connection connection;

        const TargetDescriptor& targetDescriptor;

        /**
         * When the GDB client is waiting for the target to halt, this is set to true so we know when to notify the
         * client.
         */
        bool waitingForBreak = false;

        DebugSession(const Connection& connection, const TargetDescriptor& targetDescriptor);

        void terminate();
    };
}
