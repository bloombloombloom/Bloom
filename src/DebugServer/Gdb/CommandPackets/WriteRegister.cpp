#include "WriteRegister.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/TargetStopped.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/OkResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"

#include "src/Targets/TargetRegister.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    using Targets::TargetRegister;
    using Targets::TargetRegisterDescriptors;

    using ResponsePackets::ResponsePacket;
    using ResponsePackets::OkResponsePacket;
    using ResponsePackets::ErrorResponsePacket;

    using Exceptions::Exception;

    WriteRegister::WriteRegister(const std::vector<unsigned char>& rawPacket)
        : CommandPacket(rawPacket)
    {
        // The P packet updates a single register
        auto packet = std::string(this->data.begin(), this->data.end());

        if (packet.size() < 4) {
            throw Exception("Invalid P command packet - insufficient data in packet.");
        }

        if (packet.find('=') == std::string::npos) {
            throw Exception("Invalid P command packet - unexpected format");
        }

        auto packetSegments = QString::fromStdString(packet).split("=");
        this->registerNumber = static_cast<int>(packetSegments.front().mid(1).toUInt(nullptr, 16));
        this->registerValue = Packet::hexToData(packetSegments.back().toStdString());
        std::reverse(this->registerValue.begin(), this->registerValue.end());
    }

    void WriteRegister::handle(DebugSession& debugSession, TargetControllerConsole& targetControllerConsole) {
        Logger::debug("Handling WriteRegister packet");

        try {
            auto targetRegisterDescriptor = debugSession.targetDescriptor.getTargetRegisterDescriptorFromNumber(
                this->registerNumber
            );

            const auto valueSize = this->registerValue.size();
            if (valueSize > 0 && valueSize > targetRegisterDescriptor.size) {
                // Attempt to trim the higher zero-value bytes from the register value, until we reach the correct size.
                for (auto i = this->registerValue.size() - 1; i >= targetRegisterDescriptor.size; i--) {
                    if (this->registerValue.at(i) != 0x00) {
                        // If we reach a non-zero byte, we cannot trim anymore without changing the data
                        break;
                    }

                    this->registerValue.erase(this->registerValue.begin() + i);
                }

                if (this->registerValue.size() > targetRegisterDescriptor.size) {
                    const auto& gdbRegisterDescriptor = debugSession.targetDescriptor.getRegisterDescriptorFromNumber(
                        this->registerNumber
                    );
                    throw Exception("Cannot set value for " + gdbRegisterDescriptor.name
                        + " - value size exceeds register size."
                    );
                }
            }

            targetControllerConsole.writeRegisters({
                TargetRegister(targetRegisterDescriptor, this->registerValue)
            });

            debugSession.connection.writePacket(OkResponsePacket());

        } catch (const Exception& exception) {
            Logger::error("Failed to write registers - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket());
        }
    }
}
