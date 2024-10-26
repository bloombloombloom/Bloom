#include "WriteRegister.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/OkResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"

#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::ResponsePacket;
    using ResponsePackets::OkResponsePacket;
    using ResponsePackets::ErrorResponsePacket;

    using Exceptions::Exception;

    WriteRegister::WriteRegister(const RawPacket& rawPacket)
        : Gdb::CommandPackets::CommandPacket(rawPacket)
    {
        using Services::StringService;

        if (this->data.size() < 4) {
            throw Exception{"Invalid WriteRegister command packet - insufficient data in packet."};
        }

        // The P packet updates a single register
        auto command = std::string{this->data.begin() + 1, this->data.end()};

        const auto delimiterPos = command.find_first_of('=');
        if (delimiterPos == std::string::npos) {
            throw Exception{"Invalid packet"};
        }

        this->registerId = static_cast<GdbRegisterId>(StringService::toUint32(command.substr(0, delimiterPos), 16));
        this->registerValue = Services::StringService::dataFromHex(command.substr(delimiterPos + 1));

        if (this->registerValue.empty()) {
            throw Exception{"Invalid WriteRegister command packet - missing register value"};
        }

        // LSB to MSB
        std::reverse(this->registerValue.begin(), this->registerValue.end());
    }

    void WriteRegister::handle(
        DebugSession& debugSession,
        const AvrGdbTargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor& targetDescriptor,
        TargetControllerService& targetControllerService
    ) {
        Logger::info("Handling WriteRegister packet");

        try {
            if (this->registerId == AvrGdbTargetDescriptor::PROGRAM_COUNTER_GDB_REGISTER_ID) {
                if (this->registerValue.size() != 4) {
                    throw Exception{"Invalid PC value register size"};
                }

                targetControllerService.setProgramCounter(
                    static_cast<Targets::TargetMemoryAddress>(
                        this->registerValue[0] << 24
                        | this->registerValue[1] << 16
                        | this->registerValue[2] << 8
                        | this->registerValue[3]
                    )
                );

                debugSession.connection.writePacket(OkResponsePacket{});
                return;
            }

            if (this->registerId == AvrGdbTargetDescriptor::STACK_POINTER_GDB_REGISTER_ID) {
                if (this->registerValue.size() != 2) {
                    throw Exception{"Invalid SP register value size"};
                }

                targetControllerService.setStackPointer(
                    static_cast<Targets::TargetStackPointer>(this->registerValue[0] << 8 | this->registerValue[1])
                );

                debugSession.connection.writePacket(OkResponsePacket{});
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

            targetControllerService.writeRegister(*(targetRegisterDescriptorIt->second), this->registerValue);
            debugSession.connection.writePacket(OkResponsePacket{});

        } catch (const Exception& exception) {
            Logger::error("Failed to write registers - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket{});
        }
    }
}
