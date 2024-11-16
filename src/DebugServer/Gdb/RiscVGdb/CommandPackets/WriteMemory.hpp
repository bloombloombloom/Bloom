#pragma once

#include <cstdint>
#include <optional>

#include "RiscVGdbCommandPacketInterface.hpp"
#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace DebugServer::Gdb::RiscVGdb::CommandPackets
{
    class WriteMemory
        : public RiscVGdbCommandPacketInterface
        , private Gdb::CommandPackets::CommandPacket
    {
    public:
        Targets::TargetMemoryAddress startAddress;
        Targets::TargetMemorySize bytes;
        Targets::TargetMemoryBuffer buffer;

        explicit WriteMemory(const RawPacket& rawPacket, const RiscVGdbTargetDescriptor& gdbTargetDescriptor);

        void handle(
            Gdb::DebugSession& debugSession,
            const RiscVGdbTargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;

    private:
        struct PacketData
        {
            GdbMemoryAddress gdbStartAddress;
            std::uint32_t bytes;
            Targets::TargetMemoryBuffer buffer;
        };

        static PacketData extractPacketData(const RawPacket& rawPacket);
        WriteMemory(const RawPacket& rawPacket, const RiscVGdbTargetDescriptor& gdbTargetDescriptor, PacketData&& packetData);
    };
}
