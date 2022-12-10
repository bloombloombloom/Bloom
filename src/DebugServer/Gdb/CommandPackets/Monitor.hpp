#pragma once

#include <cstdint>
#include <string>
#include <map>
#include <optional>

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

        /**
         * A mapping of any command options included in this->command. A command option must begin with "--" and
         * can optionally have a value.
         *
         * The key of this map is the option name. The map value is the option value, or std::nullopt if no value was
         * provided.
         */
        std::map<std::string, std::optional<std::string>> commandOptions;

        explicit Monitor(const RawPacket& rawPacket);

        void handle(
            DebugSession& debugSession,
            TargetController::TargetControllerConsole& targetControllerConsole
        ) override;

    private:
        /**
         * Extracts command options from a command string.
         *
         * @param command
         * @return
         */
        std::map<std::string, std::optional<std::string>> extractCommandOptions(const std::string& command);
    };
}
