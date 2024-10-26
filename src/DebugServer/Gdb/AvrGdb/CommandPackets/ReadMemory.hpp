#pragma once

#include <cstdint>
#include <optional>

#include "AvrGdbCommandPacketInterface.hpp"
#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"

#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The ReadMemory class implements a structure for "m" packets. Upon receiving these packets, the server is
     * expected to read memory from the target and send it the client.
     */
    class ReadMemory
        : public CommandPackets::AvrGdbCommandPacketInterface
        , private Gdb::CommandPackets::CommandPacket
    {
    public:
        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor;

        Targets::TargetMemoryAddress startAddress;
        Targets::TargetMemorySize bytes;

        ReadMemory(const RawPacket& rawPacket, const AvrGdbTargetDescriptor& gdbTargetDescriptor);

        void handle(
            DebugSession& debugSession,
            const AvrGdbTargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;

    private:
        struct PacketData
        {
            GdbMemoryAddress gdbStartAddress;
            std::uint32_t bytes;
        };

        static PacketData extractPacketData(const RawPacket& rawPacket);
        ReadMemory(
            const RawPacket& rawPacket,
            const AvrGdbTargetDescriptor& gdbTargetDescriptor,
            PacketData&& packetData
        );
    };
}
