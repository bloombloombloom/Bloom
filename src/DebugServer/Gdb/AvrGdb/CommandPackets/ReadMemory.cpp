#include "ReadMemory.hpp"

#include <cassert>

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"

#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"

#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::ErrorResponsePacket;
    using ResponsePackets::ResponsePacket;

    using Exceptions::Exception;

    ReadMemory::ReadMemory(const RawPacket& rawPacket, const AvrGdbTargetDescriptor& gdbTargetDescriptor)
        : ReadMemory(rawPacket, gdbTargetDescriptor, ReadMemory::extractPacketData(rawPacket))
    {}

    void ReadMemory::handle(
        DebugSession& debugSession,
        const AvrGdbTargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor& targetDescriptor,
        TargetControllerService& targetControllerService
    ) {
        Logger::info("Handling ReadMemory packet");

        try {
            if (this->bytes == 0) {
                debugSession.connection.writePacket(ResponsePacket{Targets::TargetMemoryBuffer{}});
                return;
            }

            const auto addressRange = Targets::TargetMemoryAddressRange{
                this->startAddress,
                this->startAddress + this->bytes - 1
            };

            const auto memorySegmentDescriptors = this->addressSpaceDescriptor.getIntersectingMemorySegmentDescriptors(
                addressRange
            );

            /*
             * First pass to ensure that we can read all of the memory before attempting to do so. And to ensure that
             * the requested address range completely resides within known memory segments.
             */
            auto accessibleBytes = Targets::TargetMemorySize{0};
            for (const auto* memorySegmentDescriptor : memorySegmentDescriptors) {
                if (!memorySegmentDescriptor->debugModeAccess.readable) {
                    throw Exception{
                        "Attempted to access restricted memory segment (" + memorySegmentDescriptor->key
                            + ") - segment not readable in debug mode"
                    };
                }

                accessibleBytes += memorySegmentDescriptor->addressRange.intersectingSize(addressRange);
            }

            /*
             * GDB will sometimes request an excess of up to two bytes outside the memory segment address range, even
             * though we provide it with a memory map. I don't know why it does this, but I do know that we must
             * tolerate it, otherwise GDB will moan.
             */
            if (accessibleBytes < this->bytes && (this->bytes - accessibleBytes) > 2) {
                /*
                 * GDB has requested memory that, at least partially, does not reside in any known memory segment.
                 *
                 * This could be a result of GDB being configured to generate backtraces past the main function and
                 * the internal entry point of the application. This means that GDB will attempt to walk down the stack
                 * to identify every frame. The problem is that GDB doesn't really know where the stack begins, so it
                 * probes the target by continuously issuing read memory commands until the server responds with an
                 * error.
                 *
                 * CLion seems to enable this by default. Somewhere between CLion 2021.1 and 2022.1, it began issuing
                 * the "-gdb-set backtrace past-entry on" command to GDB, at the beginning of each debug session.
                 *
                 * We don't throw an exception here, because this isn't really an error and so it's best not to report
                 * it as such. I don't think it's an error because it's expected behaviour, even though we respond to
                 * GDB with an error response.
                 */
                Logger::debug(
                    "GDB requested access to memory which does not reside within any memory segment - returning error "
                        "response"
                );
                debugSession.connection.writePacket(ErrorResponsePacket{});
                return;
            }

            auto buffer = Targets::TargetMemoryBuffer(this->bytes, 0x00);

            {
                const auto atomicSession = targetControllerService.makeAtomicSession();

                for (const auto* memorySegmentDescriptor : memorySegmentDescriptors) {
                    const auto segmentStartAddress = std::max(
                        this->startAddress,
                        memorySegmentDescriptor->addressRange.startAddress
                    );

                    const auto segmentBuffer = targetControllerService.readMemory(
                        this->addressSpaceDescriptor,
                        *memorySegmentDescriptor,
                        segmentStartAddress,
                        memorySegmentDescriptor->addressRange.intersectingSize(addressRange)
                    );

                    const auto bufferOffsetIt = buffer.begin() + (segmentStartAddress - this->startAddress);
                    assert(segmentBuffer.size() <= std::distance(bufferOffsetIt, buffer.end()));

                    std::copy(segmentBuffer.begin(), segmentBuffer.end(), bufferOffsetIt);
                }
            }

            debugSession.connection.writePacket(ResponsePacket{Services::StringService::toHex(buffer)});

        } catch (const Exception& exception) {
            Logger::error("Failed to read memory from target - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket{});
        }
    }

    ReadMemory::PacketData ReadMemory::extractPacketData(const RawPacket& rawPacket) {
        using Services::StringService;

        if (rawPacket.size() < 8) {
            throw Exception{"Invalid packet length"};
        }

        const auto command = std::string{rawPacket.begin() + 2, rawPacket.end() - 3};

        const auto delimiterPos = command.find_first_of(',');
        if (delimiterPos == std::string::npos) {
            throw Exception{"Invalid packet"};
        }

        return {
            .gdbStartAddress = StringService::toUint32(command.substr(0, delimiterPos), 16),
            .bytes = StringService::toUint32(command.substr(delimiterPos + 1), 16)
        };
    }

    ReadMemory::ReadMemory(
        const RawPacket& rawPacket,
        const AvrGdbTargetDescriptor& gdbTargetDescriptor,
        ReadMemory::PacketData&& packetData
    )
        : Gdb::CommandPackets::CommandPacket(rawPacket)
        , addressSpaceDescriptor(gdbTargetDescriptor.addressSpaceDescriptorFromGdbAddress(packetData.gdbStartAddress))
        , startAddress(gdbTargetDescriptor.translateGdbAddress(packetData.gdbStartAddress))
        , bytes(packetData.bytes)
    {}
}
