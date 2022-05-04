#include "WriteMemory.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/OkResponsePacket.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServer::Gdb::AvrGdb::CommandPackets
{
    using TargetController::TargetControllerConsole;

    using ResponsePackets::ErrorResponsePacket;
    using ResponsePackets::OkResponsePacket;

    using namespace Bloom::Exceptions;

    WriteMemory::WriteMemory(const RawPacketType& rawPacket)
        : MemoryAccessCommandPacket(rawPacket)
    {
        if (this->data.size() < 4) {
            throw Exception("Invalid packet length");
        }

        auto packetString = QString::fromLocal8Bit(
            reinterpret_cast<const char*>(this->data.data() + 1),
            static_cast<int>(this->data.size() - 1)
        );

        /*
         * The write memory ('M') packet consists of three segments, an address, a length and a buffer.
         * The address and length are separated by a comma character, and the buffer proceeds a colon character.
         */
        auto packetSegments = packetString.split(",");
        if (packetSegments.size() != 2) {
            throw Exception(
                "Unexpected number of segments in packet data: " + std::to_string(packetSegments.size())
            );
        }

        bool conversionStatus = false;
        const auto gdbStartAddress = packetSegments.at(0).toUInt(&conversionStatus, 16);

        if (!conversionStatus) {
            throw Exception("Failed to parse start address from write memory packet data");
        }

        this->memoryType = this->getMemoryTypeFromGdbAddress(gdbStartAddress);
        this->startAddress = this->removeMemoryTypeIndicatorFromGdbAddress(gdbStartAddress);

        auto lengthAndBufferSegments = packetSegments.at(1).split(":");
        if (lengthAndBufferSegments.size() != 2) {
            throw Exception(
                "Unexpected number of segments in packet data: "
                    + std::to_string(lengthAndBufferSegments.size())
            );
        }

        auto bufferSize = lengthAndBufferSegments.at(0).toUInt(&conversionStatus, 16);
        if (!conversionStatus) {
            throw Exception("Failed to parse write length from write memory packet data");
        }

        this->buffer = Packet::hexToData(lengthAndBufferSegments.at(1).toStdString());

        if (this->buffer.size() != bufferSize) {
            throw Exception("Buffer size does not match length value given in write memory packet");
        }
    }

    void WriteMemory::handle(DebugSession& debugSession, TargetControllerConsole& targetControllerConsole) {
        Logger::debug("Handling WriteMemory packet");

        try {
            const auto& memoryDescriptorsByType = debugSession.gdbTargetDescriptor.targetDescriptor.memoryDescriptorsByType;

            if (!memoryDescriptorsByType.contains(this->memoryType)) {
                throw Exception("Target does not support the requested memory type.");
            }

            if (this->memoryType == Targets::TargetMemoryType::FLASH) {
                throw Exception(
                    "GDB client requested a flash memory write - This is not currently supported by Bloom."
                );
            }

            if (this->buffer.size() == 0) {
                debugSession.connection.writePacket(OkResponsePacket());
                return;
            }

            const auto& memoryDescriptor = memoryDescriptorsByType.at(this->memoryType);

            if (
                this->startAddress < memoryDescriptor.addressRange.startAddress
                || (this->startAddress + (this->buffer.size() - 1)) > memoryDescriptor.addressRange.endAddress
            ) {
                throw Exception(
                    "GDB requested access to memory which is outside the target's memory range"
                );
            }

            targetControllerConsole.writeMemory(
                this->memoryType,
                this->startAddress,
                this->buffer
            );

            debugSession.connection.writePacket(OkResponsePacket());

        } catch (const Exception& exception) {
            Logger::error("Failed to write memory to target - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket());
        }
    }
}
