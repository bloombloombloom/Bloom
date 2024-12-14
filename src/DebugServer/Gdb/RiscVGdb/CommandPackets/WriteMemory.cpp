#include "WriteMemory.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/OkResponsePacket.hpp"

#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::RiscVGdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::ErrorResponsePacket;
    using ResponsePackets::OkResponsePacket;

    using namespace Exceptions;

    WriteMemory::WriteMemory(const RawPacket& rawPacket, const RiscVGdbTargetDescriptor& gdbTargetDescriptor)
        : WriteMemory(rawPacket, gdbTargetDescriptor, WriteMemory::extractPacketData(rawPacket))
    {}

    void WriteMemory::handle(
        Gdb::DebugSession& debugSession,
        const RiscVGdbTargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor& targetDescriptor,
        const Targets::TargetState& targetState,
        TargetControllerService& targetControllerService
    ) {
        Logger::info("Handling WriteMemory packet");

        try {
            if (this->buffer.empty()) {
                debugSession.connection.writePacket(OkResponsePacket{});
                return;
            }

            const auto& systemAddressSpaceDescriptor = gdbTargetDescriptor.systemAddressSpaceDescriptor;
            const auto addressRange = Targets::TargetMemoryAddressRange{
                this->startAddress,
                this->startAddress + static_cast<Targets::TargetMemorySize>(this->buffer.size()) - 1
            };

            const auto memorySegmentDescriptors = systemAddressSpaceDescriptor.getIntersectingMemorySegmentDescriptors(
                addressRange
            );

            auto accessibleBytes = Targets::TargetMemorySize{0};
            for (const auto* memorySegmentDescriptor : memorySegmentDescriptors) {
                if (!memorySegmentDescriptor->debugModeAccess.writeable) {
                    throw Exception{
                        "Attempted to access restricted memory segment (" + memorySegmentDescriptor->key
                            + ") - segment not writeable in debug mode"
                    };
                }

                accessibleBytes += memorySegmentDescriptor->addressRange.intersectingSize(addressRange);
            }

            if (accessibleBytes < this->bytes) {
                throw Exception{"GDB requested access to memory which does not reside within any memory segment"};
            }

            {
                const auto atomicSession = targetControllerService.makeAtomicSession();

                for (const auto* memorySegmentDescriptor : memorySegmentDescriptors) {
                    const auto segmentStartAddress = std::max(
                        this->startAddress,
                        memorySegmentDescriptor->addressRange.startAddress
                    );

                    const auto bufferOffsetIt = buffer.begin() + (segmentStartAddress - this->startAddress);
                    targetControllerService.writeMemory(
                        systemAddressSpaceDescriptor,
                        *memorySegmentDescriptor,
                        segmentStartAddress,
                        Targets::TargetMemoryBuffer{
                            bufferOffsetIt,
                            bufferOffsetIt + memorySegmentDescriptor->addressRange.intersectingSize(addressRange)
                        }
                    );
                }
            }

            debugSession.connection.writePacket(OkResponsePacket{});

        } catch (const Exception& exception) {
            Logger::error("Failed to write memory to target - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket{});
        }
    }

    WriteMemory::PacketData WriteMemory::extractPacketData(const RawPacket& rawPacket) {
        using Services::StringService;

        if (rawPacket.size() < 8) {
            throw Exception{"Invalid packet length"};
        }

        const auto command = std::string{rawPacket.begin() + 2, rawPacket.end() - 3};

        const auto commaDelimiterPos = command.find_first_of(',');
        const auto colonDelimiterPos = command.find_first_of(':');
        if (commaDelimiterPos == std::string::npos || colonDelimiterPos == std::string::npos) {
            throw Exception{"Invalid packet"};
        }

        return {
            .gdbStartAddress = StringService::toUint32(command.substr(0, commaDelimiterPos), 16),
            .bytes = StringService::toUint32(
                command.substr(commaDelimiterPos + 1, colonDelimiterPos - (commaDelimiterPos + 1)),
                16
            ),
            .buffer = StringService::dataFromHex(command.substr(colonDelimiterPos + 1))
        };
    }

    WriteMemory::WriteMemory(
        const RawPacket& rawPacket,
        const RiscVGdbTargetDescriptor&,
        PacketData&& packetData
    )
        : CommandPacket(rawPacket)
        , startAddress(packetData.gdbStartAddress)
        , bytes(packetData.bytes)
        , buffer(std::move(packetData.buffer))
    {
        if (this->buffer.size() != this->bytes) {
            throw Exception{"Buffer size does not match length value given in write memory packet"};
        }
    }
}
