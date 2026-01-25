#include "Monitor.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/EmptyResponsePacket.hpp"

#include "src/EventManager/EventManager.hpp"
#include "src/EventManager/Events/InsightActivationRequested.hpp"

#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::ResponsePacket;
    using ResponsePackets::EmptyResponsePacket;

    Monitor::Monitor(const RawPacket& rawPacket)
        : CommandPacket(rawPacket)
    {
        if (this->data.size() <= 6) {
            return;
        }

        const auto decodedCommand = Services::StringService::dataFromHex(
            std::string{this->data.begin() + 6, this->data.end()}
        );

        this->command = std::string{decodedCommand.begin(), decodedCommand.end()};
        this->command.erase(this->command.find_last_not_of(" ") + 1);

        this->commandArguments = Monitor::extractCommandArguments(this->command);
    }

    void Monitor::handle(
        DebugSession& debugSession,
        const TargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor&,
        TargetControllerService& targetControllerService
    ) {
        if (this->command == "exit") {
            targetControllerService.shutdown();

            /*
             * TargetControllerService::shutdown() will return once the TC has gone down, meaning we will have
             * properly disconnected from the target and debug tool by this point. The rest of Bloom will follow.
             */
            debugSession.connection.writePacket(
                ResponsePacket{
                    Services::StringService::toHex("Bloom is shutting down - connection will be dropped shortly.\n")
                }
            );
            return;
        }

#ifndef EXCLUDE_INSIGHT
        if (this->command == "insight") {
            EventManager::triggerEvent(std::make_shared<Events::InsightActivationRequested>());

            debugSession.connection.writePacket(
                ResponsePacket{Services::StringService::toHex("The Insight GUI will be with you shortly.\n")}
            );
            return;
        }
#endif

        const auto passthroughResponse = targetControllerService.invokeTargetPassthroughCommand(
            Targets::PassthroughCommand{.arguments = this->commandArguments}
        );

        if (passthroughResponse.has_value()) {
            debugSession.connection.writePacket(ResponsePacket{
                Services::StringService::toHex(passthroughResponse->output)
            });
            return;
        }

        Logger::error("Unknown custom GDB command (\"" + this->command + "\") received.");
        debugSession.connection.writePacket(EmptyResponsePacket{});
    }

    std::vector<std::string> Monitor::extractCommandArguments(const std::string& command) {
        auto output = std::vector<std::string>{};

        // TODO: Support escaping
        auto quoteEnabled = false;
        auto argument = std::string{};

        const auto commit = [&output, &argument] () {
            output.emplace_back(std::move(argument));
            argument.clear();
        };

        for (auto i = std::string::size_type{0}; i < command.size(); ++i) {
            const auto cmdChar = command.at(i);

            if (cmdChar == '"') {
                if (quoteEnabled) {
                    commit();
                }

                quoteEnabled = !quoteEnabled;
                continue;
            }

            if (!quoteEnabled && cmdChar == ' ') {
                commit();
                continue;
            }

            argument.push_back(cmdChar);
        }

        if (!argument.empty()) {
            commit();
        }

        return output;
    }
}
