#include "WriteRegisterBitFieldMonitor.hpp"

#include <cstdint>
#include <cmath>

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/PartialResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"

#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetRegisterGroupDescriptor.hpp"
#include "src/Targets/DynamicRegisterValue.hpp"

#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    using Services::TargetControllerService;
    using Services::StringService;

    using ResponsePackets::ErrorResponsePacket;
    using ResponsePackets::PartialResponsePacket;
    using ResponsePackets::ResponsePacket;

    using ::Exceptions::Exception;

    WriteRegisterBitFieldMonitor::WriteRegisterBitFieldMonitor(Monitor&& monitorPacket)
        : Monitor(std::move(monitorPacket))
    {}

    void WriteRegisterBitFieldMonitor::handle(
        DebugSession& debugSession,
        const TargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor& targetDescriptor,
        TargetControllerService& targetControllerService
    ) {
        Logger::info("Handling WriteRegisterBitFieldMonitor packet");

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

            auto registerGroupDescriptorOpt = std::optional<
                std::reference_wrapper<const Targets::TargetRegisterGroupDescriptor>
            >{};

            const auto registerGroupKeyProvided = argCount >= 6;
            if (registerGroupKeyProvided) {
                const auto& registerGroupKey = this->commandArguments[2];
                registerGroupDescriptorOpt = peripheralDescriptor.tryGetRegisterGroupDescriptor(registerGroupKey);
                if (!registerGroupDescriptorOpt.has_value()) {
                    throw Exception{"Unknown absolute register group key \"" + registerGroupKey + "\""};
                }

            } else {
                if (peripheralDescriptor.registerGroupDescriptorsByKey.size() != 1) {
                    throw Exception{"Absolute register group key required"};
                }

                registerGroupDescriptorOpt = peripheralDescriptor.registerGroupDescriptorsByKey.begin()->second;
            }

            const auto& registerGroupDescriptor = registerGroupDescriptorOpt->get();

            if (argCount < (registerGroupKeyProvided ? 4 : 3)) {
                throw Exception{"Register key required"};
            }

            const auto& registerKey = registerGroupKeyProvided ? this->commandArguments[3] : this->commandArguments[2];
            const auto registerDescriptorOpt = registerGroupDescriptor.tryGetRegisterDescriptor(registerKey);
            if (!registerDescriptorOpt.has_value()) {
                throw Exception{"Unknown register key \"" + registerKey + "\""};
            }

            const auto& registerDescriptor = registerDescriptorOpt->get();

            if (!registerDescriptor.access.writable) {
                throw Exception{"\"" + registerDescriptor.name + "\" register is not writeable"};
            }

            if (registerDescriptor.size > 8) {
                throw Exception{"Unsupported register size"};
            }

            if (argCount < (registerGroupKeyProvided ? 5 : 4)) {
                throw Exception{"Bit field key required"};
            }

            const auto& bitFieldKey = registerGroupKeyProvided ? this->commandArguments[4] : this->commandArguments[3];
            const auto bitFieldDescriptorOpt = registerDescriptor.tryGetBitFieldDescriptor(bitFieldKey);
            if (!bitFieldDescriptorOpt.has_value()) {
                throw Exception{"Unknown bit field key \"" + bitFieldKey + "\""};
            }

            const auto& bitFieldDescriptor = bitFieldDescriptorOpt->get();

            if (argCount < (registerGroupKeyProvided ? 6 : 5)) {
                throw Exception{"Bit field value required"};
            }

            auto rawBitFieldValue = StringService::asciiToLower(
                registerGroupKeyProvided ? this->commandArguments[5] : this->commandArguments[4]
            );
            if (rawBitFieldValue.find("0b") == 0) {
                rawBitFieldValue = rawBitFieldValue.substr(2);
            }

            if (!StringService::isBinary(rawBitFieldValue)) {
                throw Exception{"Invalid bit field value given"};
            }

            const auto bitFieldWidth = bitFieldDescriptor.width();
            if (rawBitFieldValue.size() > bitFieldWidth) {
                throw Exception{
                    "The width of the given value (" + std::to_string(rawBitFieldValue.size())
                        + " bit(s)) exceeds that of the given bit field (" + std::to_string(bitFieldWidth) + " bit(s))"
                };
            }

            const auto bitFieldValue = StringService::toUint64(rawBitFieldValue, 2);

            auto dynamicValue = Targets::DynamicRegisterValue{
                registerDescriptor,
                targetControllerService.readRegister(registerDescriptor)
            };

            debugSession.connection.writePacket(PartialResponsePacket{StringService::toHex(
                "Applying bit field value " + StringService::applyTerminalColor(
                    "0b" + rawBitFieldValue,
                    StringService::TerminalColor::DARK_YELLOW
                ) + " to \"" + bitFieldDescriptor.name + "\" bit field, to \"" + registerDescriptor.name
                + "\" register, via `" + registerDescriptor.addressSpaceKey + "` address space...\n"
            )});

            const auto initialValueHex = StringService::toHex(
                dynamicValue.value
            ).substr(16 - (registerDescriptor.size * 2));
            Logger::debug("Initial register value: 0x" + initialValueHex);
            debugSession.connection.writePacket(PartialResponsePacket{StringService::toHex(
                "Initial register value: " + StringService::applyTerminalColor(
                    "0x" + initialValueHex,
                    StringService::TerminalColor::DARK_YELLOW
                ) + "\n"
            )});
            dynamicValue.setBitField(bitFieldDescriptor, bitFieldValue);

            const auto newValueHex = StringService::toHex(
                dynamicValue.value
            ).substr(16 - (registerDescriptor.size * 2));
            Logger::debug("New register value: 0x" + newValueHex);
            debugSession.connection.writePacket(PartialResponsePacket{StringService::toHex(
                "New register value: " + StringService::applyTerminalColor(
                    "0x" + newValueHex,
                    StringService::TerminalColor::DARK_YELLOW
                ) + "\n\n"
            )});

            if (dynamicValue.value > static_cast<std::uint64_t>(std::pow(2, (registerDescriptor.size * 8)))) {
                throw Exception{
                    "The new value (0x" + newValueHex + ") exceeds the size of the register ("
                        + std::to_string(registerDescriptor.size * 8) + "-bit)"
                };
            }

            debugSession.connection.writePacket(PartialResponsePacket{StringService::toHex(
                "Writing value " + StringService::applyTerminalColor(
                    "0x" + newValueHex,
                    StringService::TerminalColor::DARK_YELLOW
                ) + " (" +  std::to_string(registerDescriptor.size * 8) + "-bit"  + ") to \"" + registerDescriptor.name
                + "\" register, at address " + StringService::applyTerminalColor(
                    "0x" + StringService::toHex(registerDescriptor.startAddress),
                    StringService::TerminalColor::BLUE
                ) + ", via `" + registerDescriptor.addressSpaceKey + "` address space...\n"
            )});

            targetControllerService.writeRegister(registerDescriptor, dynamicValue.data());
            debugSession.connection.writePacket(ResponsePacket{StringService::toHex("Register written\n")});

        } catch (const std::invalid_argument& exception) {
            debugSession.connection.writePacket(ResponsePacket{
                StringService::toHex(
                    StringService::applyTerminalColor(
                        "Error: Invalid bit field value given\n",
                        StringService::TerminalColor::DARK_RED
                    )
                )
            });

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
}
