#include "ReadRegistersMonitor.hpp"

#include <bitset>

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/PartialResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"

#include "src/Services/StringService.hpp"
#include "src/Services/IntegerService.hpp"
#include "src/Logger/Logger.hpp"

#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    using Services::TargetControllerService;
    using Services::StringService;
    using Services::IntegerService;

    using ResponsePackets::ErrorResponsePacket;
    using ResponsePackets::PartialResponsePacket;
    using ResponsePackets::ResponsePacket;

    using ::Exceptions::Exception;

    ReadRegistersMonitor::ReadRegistersMonitor(Monitor&& monitorPacket)
        : Monitor(std::move(monitorPacket))
    {}

    void ReadRegistersMonitor::handle(
        DebugSession& debugSession,
        const TargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor& targetDescriptor,
        TargetControllerService& targetControllerService
    ) {
        Logger::info("Handling ReadRegisterMonitor packet");

        try {
            const auto argCount = this->commandArguments.size();
            if (argCount < 2) {
                throw Exception{"Peripheral key required"};
            }

            const auto& peripheralKey = this->commandArguments[1];
            const auto peripheralDescriptorOpt = targetDescriptor.tryGetPeripheralDescriptor(peripheralKey);

            if (!peripheralDescriptorOpt.has_value()) {
                throw Exception{"Unknown peripheral key \"" + peripheralKey + "\""};
            }

            const auto& peripheralDescriptor = peripheralDescriptorOpt->get();

            if (argCount < 3) {
                // Register details were not provided - read all registers in the peripheral
                this->handlePeripheralOutput(peripheralDescriptor, debugSession, targetControllerService);
                return;
            }

            const auto& registerGroupKey = this->commandArguments[2];
            auto registerKey = argCount >= 4 ? std::optional{this->commandArguments[3]} : std::nullopt;

            auto registerGroupDescriptorOpt = peripheralDescriptor.tryGetRegisterGroupDescriptor(
                registerGroupKey
            );

            if (!registerGroupDescriptorOpt.has_value()) {
                if (peripheralDescriptor.registerGroupDescriptorsByKey.size() != 1 || argCount >= 4) {
                    throw Exception{"Unknown register group key \"" + registerGroupKey + "\""};
                }

                registerGroupDescriptorOpt = peripheralDescriptor.registerGroupDescriptorsByKey.begin()->second;
                registerKey = registerGroupKey;
            }

            const auto& registerGroupDescriptor = registerGroupDescriptorOpt->get();

            if (!registerKey.has_value()) {
                debugSession.connection.writePacket(PartialResponsePacket{StringService::toHex(
                    "Reading all registers in " + registerGroupDescriptor.name + " register group, from "
                        + peripheralDescriptor.name + " peripheral...\n\n"
                )});
                this->handleRegisterGroupOutput(registerGroupDescriptor, debugSession, targetControllerService);
                debugSession.connection.writePacket(ResponsePacket{StringService::toHex("\n")});
                return;
            }

            const auto registerDescriptorOpt = registerGroupDescriptor.tryGetRegisterDescriptor(*registerKey);

            if (!registerDescriptorOpt.has_value()) {
                throw Exception{"Unknown register key \"" + *registerKey + "\""};
            }

            this->handleSingleRegisterOutput(registerDescriptorOpt->get(), debugSession, targetControllerService);

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

    void ReadRegistersMonitor::handleSingleRegisterOutput(
        const Targets::TargetRegisterDescriptor& registerDescriptor,
        DebugSession& debugSession,
        TargetControllerService& targetControllerService
    ) {
        auto output = std::string{"\nName: " + registerDescriptor.name + "\n"};
        output += "Address: " + StringService::applyTerminalColor(
            "0x" + StringService::asciiToUpper(StringService::toHex(registerDescriptor.startAddress)),
            StringService::TerminalColor::DARK_BLUE
        ) + "\n";
        output += "Size: " + std::to_string(registerDescriptor.size) + " byte(s)\n\n";

        output += "----------- Value -----------\n";

        if (registerDescriptor.access.readable) {
            const auto value = IntegerService::toUint64(
                targetControllerService.readRegister(registerDescriptor)
            );

            output += StringService::applyTerminalColor(
                "0x" + StringService::asciiToUpper(StringService::toHex(value)).substr(16 - (registerDescriptor.size * 2)),
                StringService::TerminalColor::DARK_YELLOW
            );
            output += " (" + StringService::applyTerminalColor(
                std::to_string(value),
                StringService::TerminalColor::DARK_YELLOW
            );
            output += ", " + StringService::applyTerminalColor(
                "0b" + std::bitset<64>{value}.to_string().substr(64 - (registerDescriptor.size * 8)),
                StringService::TerminalColor::DARK_YELLOW
            ) + ")\n";

            for (const auto& [bitFieldKey, bitFieldDescriptor] : registerDescriptor.bitFieldDescriptorsByKey) {
                output += bitFieldDescriptor.name + ": " + StringService::applyTerminalColor(
                    "0b" + StringService::toBinaryStringWithMask(value, bitFieldDescriptor.mask),
                    StringService::TerminalColor::DARK_YELLOW
                ) + " ";

                // TODO: Warn about bit field value being meaningless if the bit field isn't readable
            }

        } else {
            output += StringService::applyTerminalColor("inaccessible\n", StringService::TerminalColor::DARK_RED);
        }

        output += "\n";
        debugSession.connection.writePacket(ResponsePacket{StringService::toHex(output)});
    }

    void ReadRegistersMonitor::handlePeripheralOutput(
        const Targets::TargetPeripheralDescriptor& peripheralDescriptor,
        DebugSession& debugSession,
        TargetControllerService& targetControllerService
    ) {
        debugSession.connection.writePacket(PartialResponsePacket{StringService::toHex(
            "Reading " + peripheralDescriptor.name + " peripheral registers...\n\n"
        )});

        for (const auto& [groupKey, groupDescriptor] : peripheralDescriptor.registerGroupDescriptorsByKey) {
            this->handleRegisterGroupOutput(groupDescriptor, debugSession, targetControllerService);
        }

        debugSession.connection.writePacket(ResponsePacket{StringService::toHex("\n")});
    }

    void ReadRegistersMonitor::handleRegisterGroupOutput(
        const Targets::TargetRegisterGroupDescriptor& groupDescriptor,
        DebugSession& debugSession,
        TargetControllerService& targetControllerService
    ) {
        for (const auto& [registerKey, registerDescriptor] : groupDescriptor.registerDescriptorsByKey) {
            auto output = std::string{registerDescriptor.absoluteGroupKey + ", "};
            output += registerDescriptor.key + ", ";
            output += registerDescriptor.name + ", ";
            output += StringService::applyTerminalColor(
                "0x" + StringService::asciiToUpper(StringService::toHex(registerDescriptor.startAddress)),
                StringService::TerminalColor::DARK_BLUE
            ) + ", ";
            output += std::to_string(registerDescriptor.size) + " byte(s) | ";

            if (registerDescriptor.access.readable) {
                const auto value = IntegerService::toUint64(
                    targetControllerService.readRegister(registerDescriptor)
                );

                output += StringService::applyTerminalColor(
                    "0x" + StringService::asciiToUpper(StringService::toHex(value)).substr(16 - (registerDescriptor.size * 2)),
                    StringService::TerminalColor::DARK_YELLOW
                );
                output += " (" + StringService::applyTerminalColor(
                    std::to_string(value),
                    StringService::TerminalColor::DARK_YELLOW
                );
                output += ", " + StringService::applyTerminalColor(
                    "0b" + std::bitset<64>{value}.to_string().substr(64 - (registerDescriptor.size * 8)),
                    StringService::TerminalColor::DARK_YELLOW
                ) + ")";

                for (const auto& [bitFieldKey, bitFieldDescriptor] : registerDescriptor.bitFieldDescriptorsByKey) {
                    output += ", " + bitFieldDescriptor.name + ": " + StringService::applyTerminalColor(
                        "0b" + StringService::toBinaryStringWithMask(value, bitFieldDescriptor.mask),
                        StringService::TerminalColor::DARK_YELLOW
                    );
                }

            } else {
                output += StringService::applyTerminalColor("inaccessible", StringService::TerminalColor::DARK_RED);
            }

            output += "\n";
            debugSession.connection.writePacket(PartialResponsePacket{StringService::toHex(output)});
        }

        for (const auto& [subGroupKey, subGroupDescriptor] : groupDescriptor.subgroupDescriptorsByKey) {
            this->handleRegisterGroupOutput(subGroupDescriptor, debugSession, targetControllerService);
        }
    }
}
