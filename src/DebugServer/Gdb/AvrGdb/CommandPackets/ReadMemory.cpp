#include "ReadMemory.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServer::Gdb::AvrGdb::CommandPackets
{
    using TargetController::TargetControllerConsole;

    using ResponsePackets::ErrorResponsePacket;
    using ResponsePackets::ResponsePacket;

    using Exceptions::Exception;

    ReadMemory::ReadMemory(const RawPacket& rawPacket, const TargetDescriptor& gdbTargetDescriptor)
        : CommandPacket(rawPacket)
    {
        if (this->data.size() < 4) {
            throw Exception("Invalid packet length");
        }

        const auto packetString = QString::fromLocal8Bit(
            reinterpret_cast<const char*>(this->data.data() + 1),
            static_cast<int>(this->data.size() - 1)
        );

        /*
         * The read memory ('m') packet consists of two segments, an address and a number of bytes to read.
         * These are separated by a comma character.
         */
        const auto packetSegments = packetString.split(",");

        if (packetSegments.size() != 2) {
            throw Exception(
                "Unexpected number of segments in packet data: " + std::to_string(packetSegments.size())
            );
        }

        bool conversionStatus = false;
        const auto gdbStartAddress = packetSegments.at(0).toUInt(&conversionStatus, 16);

        if (!conversionStatus) {
            throw Exception("Failed to parse start address from read memory packet data");
        }

        /*
         * Extract the memory type from the memory address (see Gdb::TargetDescriptor::memoryOffsetsByType for more on
         * this).
         */
        this->memoryType = gdbTargetDescriptor.getMemoryTypeFromGdbAddress(gdbStartAddress);
        this->startAddress = gdbStartAddress & ~(gdbTargetDescriptor.getMemoryOffset(this->memoryType));

        this->bytes = packetSegments.at(1).toUInt(&conversionStatus, 16);

        if (!conversionStatus) {
            throw Exception("Failed to parse read length from read memory packet data");
        }
    }

    void ReadMemory::handle(DebugSession& debugSession, TargetControllerConsole& targetControllerConsole) {
        Logger::debug("Handling ReadMemory packet");

        try {
            const auto& memoryDescriptorsByType = debugSession.gdbTargetDescriptor.targetDescriptor.memoryDescriptorsByType;

            if (!memoryDescriptorsByType.contains(this->memoryType)) {
                throw Exception("Target does not support the requested memory type.");
            }

            if (this->bytes == 0) {
                debugSession.connection.writePacket(ResponsePacket(std::vector<unsigned char>()));
                return;
            }

            const auto& memoryDescriptor = memoryDescriptorsByType.at(this->memoryType);

            /*
             * In AVR targets, RAM is mapped to many registers and peripherals - we don't want to block GDB from
             * accessing them.
             */
            const auto permittedStartAddress = (this->memoryType == Targets::TargetMemoryType::RAM)
                ? 0x00
                : memoryDescriptor.addressRange.startAddress;

            const auto permittedEndAddress = memoryDescriptor.addressRange.endAddress + 2;

            if (
                this->startAddress < permittedStartAddress
                || (this->startAddress + (this->bytes - 1)) > permittedEndAddress
            ) {
                /*
                 * GDB can be configured to generate backtraces past the main function and the internal entry point
                 * of the application. Although this isn't very useful to most devs, CLion now seems to enable it by
                 * default. Somewhere between CLion 2021.1 and 2022.1, it began issuing the "-gdb-set backtrace past-entry on"
                 * command to GDB, at the beginning of each debug session.
                 *
                 * This means that GDB will attempt to walk down the stack to identify every frame. The problem is that
                 * GDB doesn't really know where the stack begins, so it ends up in a loop, continually issuing read
                 * memory commands. This has exposed an issue on our end - we need to validate the requested memory
                 * address range and reject any request for a range that's not within the target's memory. We do this
                 * here.
                 *
                 * We don't throw an exception here, because this isn't really an error and so it's best not to report
                 * it as such. I don't think it's an error because it's expected behaviour, even though we respond to
                 * GDB with an error response.
                 */
                Logger::debug(
                    "GDB requested access to memory which is outside the target's memory range - returning error response"
                );
                debugSession.connection.writePacket(ErrorResponsePacket());
                return;
            }

            /*
             * GDB may request more bytes than what's available (even though we give it a memory map?!) - ensure that
             * we don't try to read any more than what's available.
             *
             * We fill the out-of-bounds bytes with 0x00, below.
             */
            const auto bytesToRead = (this->startAddress <= memoryDescriptor.addressRange.endAddress)
                ? std::min(this->bytes, (memoryDescriptor.addressRange.endAddress - this->startAddress) + 1)
                : 0;

            auto memoryBuffer = Targets::TargetMemoryBuffer();

            if (bytesToRead > 0) {
                memoryBuffer = targetControllerConsole.readMemory(
                    this->memoryType,
                    this->startAddress,
                    bytesToRead
                );
            }

            if (bytesToRead < this->bytes) {
                // GDB requested some out-of-bounds memory - fill the inaccessible bytes with 0x00
                memoryBuffer.insert(memoryBuffer.end(), (this->bytes - bytesToRead), 0x00);
            }

            debugSession.connection.writePacket(
                ResponsePacket(Packet::toHex(memoryBuffer))
            );

        } catch (const Exception& exception) {
            Logger::error("Failed to read memory from target - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket());
        }
    }
}
