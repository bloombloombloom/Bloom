#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "CommandPacket.hpp"

namespace DebugServer::Gdb::CommandPackets
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

        std::vector<std::string> commandArguments;

        explicit Monitor(const RawPacket& rawPacket);

        void handle(
            DebugSession& debugSession,
            const TargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;

    private:
        static std::vector<std::string> extractCommandArguments(const std::string& command);
    };
}
