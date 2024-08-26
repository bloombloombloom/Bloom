#include "ListRegistersMonitor.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/PartialResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"

#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"

#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    using Services::TargetControllerService;
    using Services::StringService;

    using ResponsePackets::ErrorResponsePacket;
    using ResponsePackets::PartialResponsePacket;
    using ResponsePackets::ResponsePacket;

    using ::Exceptions::Exception;

    ListRegistersMonitor::ListRegistersMonitor(Monitor&& monitorPacket)
        : Monitor(std::move(monitorPacket))
    {}

    void ListRegistersMonitor::handle(
        DebugSession& debugSession,
        const TargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor& targetDescriptor,
        TargetControllerService& targetControllerService
    ) {
        Logger::info("Handling ReadRegisterMonitor packet");

        try {
            const auto argCount = this->commandArguments.size();
            if (argCount < 2) {
                for (const auto& [peripheralKey, peripheralDescriptor] : targetDescriptor.peripheralDescriptorsByKey) {
                    this->handlePeripheralOutput(peripheralDescriptor, debugSession);
                    debugSession.connection.writePacket(PartialResponsePacket{StringService::toHex("\n")});
                }

                debugSession.connection.writePacket(ResponsePacket{StringService::toHex("\n")});
                return;
            }

            const auto& peripheralKey = this->commandArguments[1];
            const auto peripheralDescriptorOpt = targetDescriptor.tryGetPeripheralDescriptor(peripheralKey);

            if (!peripheralDescriptorOpt.has_value()) {
                throw Exception{"Unknown peripheral key \"" + peripheralKey + "\""};
            }

            const auto& peripheralDescriptor = peripheralDescriptorOpt->get();

            if (argCount < 3) {
                this->handlePeripheralOutput(peripheralDescriptor, debugSession);
                debugSession.connection.writePacket(ResponsePacket{StringService::toHex("\n")});

                return;
            }

            const auto& registerGroupKey = this->commandArguments[2];

            auto registerGroupDescriptorOpt = peripheralDescriptor.tryGetRegisterGroupDescriptor(
                registerGroupKey
            );

            if (!registerGroupDescriptorOpt.has_value()) {
                throw Exception{"Unknown register group key \"" + registerGroupKey + "\""};
            }

            const auto& registerGroupDescriptor = registerGroupDescriptorOpt->get();

            debugSession.connection.writePacket(PartialResponsePacket{StringService::toHex(
                peripheralDescriptor.name + " peripheral registers, in " + registerGroupDescriptor.name
                    + " register group:\n\n"
            )});
            this->handleRegisterGroupOutput(registerGroupDescriptor, debugSession);
            debugSession.connection.writePacket(ResponsePacket{StringService::toHex("\n")});

        } catch (const Exception& exception) {
            debugSession.connection.writePacket(ResponsePacket{
                StringService::toHex(
                    StringService::applyTerminalColor(
                        "Error: " + exception.getMessage() + "\n",
                        StringService::TerminalColor::DARK_RED
                    )
                )
            });
        }
    }

    void ListRegistersMonitor::handlePeripheralOutput(
        const Targets::TargetPeripheralDescriptor& peripheralDescriptor,
        DebugSession& debugSession
    ) {
        debugSession.connection.writePacket(PartialResponsePacket{StringService::toHex(
            "---------- " + StringService::applyTerminalColor(
                peripheralDescriptor.name,
                StringService::TerminalColor::DARK_GREEN
            ) + " (" + StringService::applyTerminalColor(
                peripheralDescriptor.key,
                StringService::TerminalColor::DARK_YELLOW
            ) + ") peripheral registers ----------\n\n"
        )});

        for (const auto& [groupKey, groupDescriptor] : peripheralDescriptor.registerGroupDescriptorsByKey) {
            this->handleRegisterGroupOutput(groupDescriptor, debugSession);
        }
    }

    void ListRegistersMonitor::handleRegisterGroupOutput(
        const Targets::TargetRegisterGroupDescriptor& groupDescriptor,
        DebugSession& debugSession
    ) {
        for (const auto& [registerKey, registerDescriptor] : groupDescriptor.registerDescriptorsByKey) {
            auto output = std::string{registerDescriptor.absoluteGroupKey + ", "};
            output += registerDescriptor.key + ", ";
            output += registerDescriptor.name + ", ";
            output += StringService::applyTerminalColor(
                "0x" + StringService::asciiToUpper(StringService::toHex(registerDescriptor.startAddress)),
                StringService::TerminalColor::DARK_BLUE
            ) + ", ";
            output += std::to_string(registerDescriptor.size) + " byte(s)";

            if (registerDescriptor.description.has_value()) {
                output += ", \"" + *(registerDescriptor.description) + "\"";
            }

            output += "\n";
            debugSession.connection.writePacket(PartialResponsePacket{StringService::toHex(output)});
        }

        for (const auto& [subGroupKey, subGroupDescriptor] : groupDescriptor.subgroupDescriptorsByKey) {
            this->handleRegisterGroupOutput(subGroupDescriptor, debugSession);
        }
    }
}
