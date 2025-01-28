#include "ReadMemory.hpp"

#include <cassert>

#include "src/Targets/TargetMemoryAddressRange.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"

#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"

#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::RiscVGdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::ErrorResponsePacket;
    using ResponsePackets::ResponsePacket;

    using Exceptions::Exception;

    ReadMemory::ReadMemory(const RawPacket& rawPacket, const RiscVGdbTargetDescriptor& gdbTargetDescriptor)
        : ReadMemory(rawPacket, gdbTargetDescriptor, ReadMemory::extractPacketData(rawPacket))
    {}

    void ReadMemory::handle(
        Gdb::DebugSession& debugSession,
        const RiscVGdbTargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor& targetDescriptor,
        const Targets::TargetState& targetState,
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

            if (accessibleBytes < this->bytes) {
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
        const RiscVGdbTargetDescriptor& gdbTargetDescriptor,
        ReadMemory::PacketData&& packetData
    )
        : CommandPacket(rawPacket)
        , addressSpaceDescriptor(gdbTargetDescriptor.systemAddressSpaceDescriptor)
        , startAddress(packetData.gdbStartAddress)
        , bytes(packetData.bytes)
    {}
}
