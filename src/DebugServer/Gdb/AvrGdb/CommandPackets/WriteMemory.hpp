#pragma once

#include <cstdint>
#include <optional>

#include "CommandPacket.hpp"

#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The WriteMemory class implements the structure for "M" packets. Upon receiving this packet, the server is
     * expected to write data to the target's memory, at the specified start address.
     */
    class WriteMemory: public CommandPackets::CommandPacket
    {
    public:
        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor;
        Targets::TargetMemoryAddress startAddress;
        Targets::TargetMemorySize bytes;
        Targets::TargetMemoryBuffer buffer;

        explicit WriteMemory(const RawPacket& rawPacket, const AvrGdbTargetDescriptor& gdbTargetDescriptor);

        void handle(
            DebugSession& debugSession,
            const AvrGdbTargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;

    private:
        struct PacketData
        {
            std::uint32_t gdbStartAddress;
            std::uint32_t bytes;
            Targets::TargetMemoryBuffer buffer;
        };

        static PacketData extractPacketData(const RawPacket& rawPacket);
        WriteMemory(const RawPacket& rawPacket, const AvrGdbTargetDescriptor& gdbTargetDescriptor, PacketData&& packetData);
    };
}
