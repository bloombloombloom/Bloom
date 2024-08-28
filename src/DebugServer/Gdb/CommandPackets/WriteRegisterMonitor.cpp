#include "WriteRegisterMonitor.hpp"

#include <cstdint>
#include <cmath>

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/PartialResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"

#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetRegisterGroupDescriptor.hpp"

#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    using Services::TargetControllerService;
    using Services::StringService;

    using ResponsePackets::ErrorResponsePacket;
    using ResponsePackets::PartialResponsePacket;
    using ResponsePackets::ResponsePacket;

    using ::Exceptions::Exception;

    WriteRegisterMonitor::WriteRegisterMonitor(Monitor&& monitorPacket)
        : Monitor(std::move(monitorPacket))
    {}

    void WriteRegisterMonitor::handle(
        DebugSession& debugSession,
        const TargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor& targetDescriptor,
        TargetControllerService& targetControllerService
    ) {
        Logger::info("Handling WriteRegisterMonitor packet");

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

            const auto registerGroupKeyProvided = argCount >= 5;
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
                throw Exception{registerDescriptor.name + " register is not writeable"};
            }

            if (registerDescriptor.size > 8) {
                throw Exception{"Unsupported register size"};
            }

            if (argCount < (registerGroupKeyProvided ? 5 : 4)) {
                throw Exception{"Register value required"};
            }

            const auto& rawValue = registerGroupKeyProvided ? this->commandArguments[4] : this->commandArguments[3];
            const auto value = StringService::toUint64(rawValue, 16);

            if (value > static_cast<std::uint64_t>(std::pow(2, (registerDescriptor.size * 8)))) {
                throw Exception{
                    "The given value (0x" + StringService::toHex(value) + ") exceeds the size of the register ("
                        + std::to_string(registerDescriptor.size) + " byte(s))"
                };
            }

            auto buffer = Targets::TargetMemoryBuffer{};
            buffer.reserve(registerDescriptor.size);

            for (auto i = std::uint8_t{0}; i < registerDescriptor.size; ++i) {
                buffer.insert(buffer.begin(), static_cast<unsigned char>(value >> (registerDescriptor.size * 8 * i)));
            }

            debugSession.connection.writePacket(PartialResponsePacket{StringService::toHex(
                "Writing value " + StringService::applyTerminalColor(
                    "0x" + StringService::toHex(value).substr(16 - (registerDescriptor.size * 2)),
                    StringService::TerminalColor::DARK_YELLOW
                ) + " (" +  std::to_string(buffer.size()) + " byte(s)"  + ") to " + registerDescriptor.name
                + " register, at address " + StringService::applyTerminalColor(
                    "0x" + StringService::toHex(registerDescriptor.startAddress),
                    StringService::TerminalColor::BLUE
                ) + ", via `" + registerDescriptor.addressSpaceKey + "` address space...\n"
            )});

            targetControllerService.writeRegister(registerDescriptor, buffer);

            debugSession.connection.writePacket(ResponsePacket{StringService::toHex("Register written\n")});

        } catch (const std::invalid_argument& exception) {
            debugSession.connection.writePacket(ResponsePacket{
                StringService::toHex(
                    StringService::applyTerminalColor(
                        "Error: Invalid register value given\n",
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
