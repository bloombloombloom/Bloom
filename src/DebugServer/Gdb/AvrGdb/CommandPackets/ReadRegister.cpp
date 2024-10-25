#include "ReadRegister.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"

#include "src/Targets/TargetRegisterDescriptor.hpp"

#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"

#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    using Services::TargetControllerService;

    using Targets::TargetRegisterDescriptors;

    using ResponsePackets::ResponsePacket;
    using ResponsePackets::ErrorResponsePacket;

    using Exceptions::Exception;

    ReadRegister::ReadRegister(const RawPacket& rawPacket)
        : CommandPacket(rawPacket)
    {
        using Services::StringService;

        if (this->data.size() < 2) {
            throw Exception{"Invalid packet length"};
        }

        this->registerId = static_cast<GdbRegisterId>(
            StringService::toUint32(std::string{this->data.begin() + 1, this->data.end()}, 16)
        );
    }

    void ReadRegister::handle(
        DebugSession& debugSession,
        const AvrGdbTargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor& targetDescriptor,
        TargetControllerService& targetControllerService
    ) {
        Logger::info("Handling ReadRegister packet");

        try {
            Logger::debug("Reading GDB register ID: " + std::to_string(this->registerId));

            if (this->registerId == AvrGdbTargetDescriptor::PROGRAM_COUNTER_GDB_REGISTER_ID) {
                /*
                 * GDB has requested the program counter. We can't access this in the same way as we do with other
                 * registers.
                 */
                const auto programCounter = targetControllerService.getProgramCounter();

                debugSession.connection.writePacket(
                    ResponsePacket{Services::StringService::toHex(Targets::TargetMemoryBuffer{
                        static_cast<unsigned char>(programCounter),
                        static_cast<unsigned char>(programCounter >> 8),
                        static_cast<unsigned char>(programCounter >> 16),
                        static_cast<unsigned char>(programCounter >> 24),
                    })}
                );

                return;
            }

            if (this->registerId == AvrGdbTargetDescriptor::STACK_POINTER_GDB_REGISTER_ID) {
                /*
                 * GDB has requested the program counter. We can't access this in the same way as we do with other
                 * registers.
                 */
                const auto stackPointer = targetControllerService.getStackPointer();

                debugSession.connection.writePacket(
                    ResponsePacket{Services::StringService::toHex(Targets::TargetMemoryBuffer{
                        static_cast<unsigned char>(stackPointer),
                        static_cast<unsigned char>(stackPointer >> 8),
                    })}
                );

                return;
            }

            const auto gdbRegisterDescriptorIt = gdbTargetDescriptor.gdbRegisterDescriptorsById.find(this->registerId);
            const auto targetRegisterDescriptorIt = gdbTargetDescriptor.targetRegisterDescriptorsByGdbId.find(
                this->registerId
            );

            if (
                gdbRegisterDescriptorIt == gdbTargetDescriptor.gdbRegisterDescriptorsById.end()
                || targetRegisterDescriptorIt == gdbTargetDescriptor.targetRegisterDescriptorsByGdbId.end()
            ) {
                throw Exception{"Unknown GDB register ID (" + std::to_string(this->registerId) + ")"};
            }

            auto registerValue = targetControllerService.readRegister(*(targetRegisterDescriptorIt->second));
            std::reverse(registerValue.begin(), registerValue.end()); // MSB to LSB

            const auto& gdbRegisterDescriptor = gdbRegisterDescriptorIt->second;
            if (registerValue.size() < gdbRegisterDescriptor.size) {
                // The register on the target is smaller than the size expected by GDB.
                registerValue.insert(registerValue.end(), (gdbRegisterDescriptor.size - registerValue.size()), 0x00);
            }

            debugSession.connection.writePacket(ResponsePacket{Services::StringService::toHex(registerValue)});

        } catch (const Exception& exception) {
            Logger::error("Failed to read general registers - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket{});
        }
    }
}
