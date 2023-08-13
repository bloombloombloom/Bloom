#include "ReadRegister.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/DebugServer/Gdb/AvrGdb/TargetDescriptor.hpp"

#include "src/Targets/TargetRegister.hpp"

#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"

#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    using Services::TargetControllerService;

    using Targets::TargetRegister;
    using Targets::TargetRegisterDescriptors;

    using ResponsePackets::ResponsePacket;
    using ResponsePackets::ErrorResponsePacket;

    using Exceptions::Exception;

    ReadRegister::ReadRegister(const RawPacket& rawPacket)
        : CommandPacket(rawPacket)
    {
        if (this->data.size() < 2) {
            throw Exception("Invalid packet length");
        }

        this->registerId = static_cast<GdbRegisterId>(
            std::stoi(std::string(this->data.begin() + 1, this->data.end()))
        );
    }

    void ReadRegister::handle(DebugSession& debugSession, TargetControllerService& targetControllerService) {
        Logger::info("Handling ReadRegister packet");

        try {
            Logger::debug("Reading GDB register ID: " + std::to_string(this->registerId));

            if (this->registerId == TargetDescriptor::PROGRAM_COUNTER_GDB_REGISTER_ID) {
                /*
                 * GDB has requested the program counter. We can't access this in the same way as we do with other
                 * registers.
                 */
                const auto programCounter = targetControllerService.getProgramCounter();

                debugSession.connection.writePacket(
                    ResponsePacket(Services::StringService::toHex(Targets::TargetMemoryBuffer({
                        static_cast<unsigned char>(programCounter),
                        static_cast<unsigned char>(programCounter >> 8),
                        static_cast<unsigned char>(programCounter >> 16),
                        static_cast<unsigned char>(programCounter >> 24),
                    })))
                );

                return;
            }

            const auto& targetDescriptor = debugSession.gdbTargetDescriptor;
            const auto& gdbRegisterDescriptor = targetDescriptor.gdbRegisterDescriptorsById.at(this->registerId);

            const auto targetRegisterDescriptorId = targetDescriptor.getTargetRegisterDescriptorIdFromGdbRegisterId(
                this->registerId
            );

            if (!targetRegisterDescriptorId.has_value()) {
                throw Exception("GDB requested an invalid/unknown register");
            }

            auto registerValue = targetControllerService.readRegisters({*targetRegisterDescriptorId}).front().value;

            // GDB expects register values to be in LSB.
            std::reverse(registerValue.begin(), registerValue.end());

            if (registerValue.size() < gdbRegisterDescriptor.size) {
                /*
                 * The register on the target is smaller than the size expected by GDB.
                 *
                 * Insert the rest of the bytes.
                 */
                registerValue.insert(registerValue.end(), (gdbRegisterDescriptor.size - registerValue.size()), 0x00);
            }

            debugSession.connection.writePacket(
                ResponsePacket(Services::StringService::toHex(registerValue))
            );

        } catch (const Exception& exception) {
            Logger::error("Failed to read general registers - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket());
        }
    }
}
