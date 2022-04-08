#pragma once

#include <cstdint>

#include "CommandPacket.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    /**
     * This is a base class for 'qRcmd' packets - invoked by the GDB 'monitor' command.
     */
    class Monitor: public CommandPacket
    {
    public:
        /**
         * The decoded command string which was input by the GDB user.
         */
        std::string command;

        explicit Monitor(const std::vector<unsigned char>& rawPacket);

        void handle(DebugSession& debugSession, TargetControllerConsole& targetControllerConsole) override;
    };
}
