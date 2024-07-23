#pragma once

#include <cstdint>
#include <optional>

#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"

#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemory.hpp"
#include "src/DebugServer/Gdb/AvrGdb/TargetDescriptor.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The WriteMemory class implements the structure for "M" packets. Upon receiving this packet, the server is
     * expected to write data to the target's memory, at the specified start address.
     */
    class WriteMemory: public Gdb::CommandPackets::CommandPacket
    {
    public:
        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor;
        Targets::TargetMemoryAddress startAddress;
        Targets::TargetMemorySize bytes;
        Targets::TargetMemoryBuffer buffer;

        explicit WriteMemory(const RawPacket& rawPacket, const TargetDescriptor& gdbTargetDescriptor);

        void handle(
            Gdb::DebugSession& debugSession,
            const Gdb::TargetDescriptor& gdbTargetDescriptor,
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
        WriteMemory(const RawPacket& rawPacket, const Gdb::TargetDescriptor& gdbTargetDescriptor, PacketData&& packetData);
    };
}
