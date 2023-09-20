#include "WriteRegister.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/OkResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"

#include "src/DebugServer/Gdb/AvrGdb/TargetDescriptor.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    using Services::TargetControllerService;

    using Targets::TargetRegister;
    using Targets::TargetRegisterDescriptors;

    using ResponsePackets::ResponsePacket;
    using ResponsePackets::OkResponsePacket;
    using ResponsePackets::ErrorResponsePacket;

    using Exceptions::Exception;

    WriteRegister::WriteRegister(const RawPacket& rawPacket)
        : CommandPacket(rawPacket)
    {
        // The P packet updates a single register
        auto packet = std::string(this->data.begin(), this->data.end());

        if (packet.size() < 4) {
            throw Exception("Invalid WriteRegister command packet - insufficient data in packet.");
        }

        if (packet.find('=') == std::string::npos) {
            throw Exception("Invalid WriteRegister command packet - unexpected format");
        }

        const auto packetSegments = QString::fromStdString(packet).split("=");
        this->registerId = static_cast<GdbRegisterId>(packetSegments.front().mid(1).toUInt(nullptr, 16));
        this->registerValue = Packet::hexToData(packetSegments.back().toStdString());

        if (this->registerValue.empty()) {
            throw Exception("Invalid WriteRegister command packet - missing register value");
        }

        std::reverse(this->registerValue.begin(), this->registerValue.end());
    }

    void WriteRegister::handle(Gdb::DebugSession& debugSession, TargetControllerService& targetControllerService) {
        Logger::info("Handling WriteRegister packet");

        try {
            if (this->registerId == TargetDescriptor::PROGRAM_COUNTER_GDB_REGISTER_ID) {
                targetControllerService.setProgramCounter(
                    static_cast<Targets::TargetMemoryAddress>(
                        (this->registerValue.size() >= 1 ? this->registerValue[0] : 0x00) << 24
                        | (this->registerValue.size() >= 2 ? this->registerValue[1] : 0x00) << 16
                        | (this->registerValue.size() >= 3 ? this->registerValue[2] : 0x00) << 8
                        | (this->registerValue.size() >= 4 ? this->registerValue[3] : 0x00)
                    )
                );

                debugSession.connection.writePacket(OkResponsePacket());
                return;
            }

            const auto& gdbTargetDescriptor = debugSession.gdbTargetDescriptor;
            const auto descriptorId = gdbTargetDescriptor.getTargetRegisterDescriptorIdFromGdbRegisterId(
                this->registerId
            );

            if (!descriptorId.has_value()) {
                throw Exception("Invalid/unknown register");
            }

            const auto& descriptor = gdbTargetDescriptor.targetDescriptor.registerDescriptorsById.at(*descriptorId);

            if (this->registerValue.size() > descriptor.size) {
                // Attempt to trim the higher zero-value bytes from the register value, until we reach the correct size.
                for (auto i = this->registerValue.size() - 1; i >= descriptor.size; --i) {
                    if (this->registerValue.at(i) != 0x00) {
                        // If we reach a non-zero byte, we cannot trim anymore without changing the data
                        break;
                    }

                    this->registerValue.erase(this->registerValue.begin() + i);
                }

                if (this->registerValue.size() > descriptor.size) {
                    const auto& gdbRegisterDescriptor = gdbTargetDescriptor.gdbRegisterDescriptorsById.at(
                        this->registerId
                    );
                    throw Exception(
                        "Cannot set value for " + gdbRegisterDescriptor.name + " - value size exceeds register size."
                    );
                }
            }

            targetControllerService.writeRegisters({
                TargetRegister(descriptor.id, this->registerValue)
            });

            debugSession.connection.writePacket(OkResponsePacket());

        } catch (const Exception& exception) {
            Logger::error("Failed to write registers - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket());
        }
    }
}
