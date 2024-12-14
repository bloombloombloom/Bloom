#pragma once

#include <cstdint>
#include <optional>

#include "RiscVGdbCommandPacketInterface.hpp"
#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"

#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace DebugServer::Gdb::RiscVGdb::CommandPackets
{
    class ReadMemory
        : public RiscVGdbCommandPacketInterface
        , private Gdb::CommandPackets::CommandPacket
    {
    public:
        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor;

        Targets::TargetMemoryAddress startAddress;
        Targets::TargetMemorySize bytes;

        ReadMemory(const RawPacket& rawPacket, const RiscVGdbTargetDescriptor& gdbTargetDescriptor);

        void handle(
            Gdb::DebugSession& debugSession,
            const RiscVGdbTargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            const Targets::TargetState& targetState,
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
            const RiscVGdbTargetDescriptor& gdbTargetDescriptor,
            PacketData&& packetData
        );
    };
}
