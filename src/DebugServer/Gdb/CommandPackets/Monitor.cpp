#include "Monitor.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/EmptyResponsePacket.hpp"

#include "src/Logger/Logger.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    using TargetController::TargetControllerConsole;

    using ResponsePackets::EmptyResponsePacket;

    Monitor::Monitor(const RawPacket& rawPacket)
        : CommandPacket(rawPacket)
    {
        if (this->data.size() <= 6) {
            return;
        }

        const auto decodedCommand = Packet::hexToData(
            std::string(this->data.begin() + 6, this->data.end())
        );

        this->command = std::string(decodedCommand.begin(), decodedCommand.end());
        this->command.erase(this->command.find_last_not_of(" ") + 1);

        this->commandOptions = this->extractCommandOptions(this->command);
    }

    void Monitor::handle(DebugSession& debugSession, TargetControllerConsole& targetControllerConsole) {
        Logger::error("Unknown custom GDB command (\"" + this->command + "\") received.");
        debugSession.connection.writePacket(EmptyResponsePacket());
    }

    std::map<std::string, std::optional<std::string>> Monitor::extractCommandOptions(const std::string& command) {
        auto output = std::map<std::string, std::optional<std::string>>();

        for (std::string::size_type cmdIndex = 1; cmdIndex < command.size(); ++cmdIndex) {
            const auto cmdChar = command.at(cmdIndex);

            if (cmdChar == '-') {
                if (command.at(cmdIndex - 1) != '-') {
                    continue;
                }

                auto option = std::string();
                auto optionValue = std::optional<std::string>();

                bool quoted = false;

                auto optIndex = std::string::size_type(0);
                for (optIndex = cmdIndex + 1; optIndex < command.size(); ++optIndex) {
                    const auto optChar = command.at(optIndex);

                    if (!option.empty() && ((!quoted && optChar == ' ') || (quoted && optChar == '"'))) {
                        output.insert(std::pair(option, optionValue));

                        option.clear();
                        optionValue.reset();
                        quoted = false;

                        cmdIndex = optIndex;
                        break;
                    }

                    if (optionValue.has_value()) {
                        if (optChar == '"' && !quoted && optionValue->empty()) {
                            quoted = true;
                            continue;
                        }

                        optionValue->push_back(optChar);
                        continue;
                    }

                    if (optChar == '=') {
                        optionValue = std::string();
                        continue;
                    }

                    option.push_back(optChar);
                    continue;
                }

                if (!option.empty()) {
                    output.insert(std::pair(option, optionValue));
                    cmdIndex = optIndex;
                }
            }
        }

        return output;
    }
}
