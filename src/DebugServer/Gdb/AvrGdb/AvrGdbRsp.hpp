#pragma once

#include <cstdint>

#include "AvrGdbTargetDescriptor.hpp"
#include "CommandPackets/CommandPacket.hpp"

#include "src/DebugServer/Gdb/GdbRspDebugServer.hpp"

namespace DebugServer::Gdb::AvrGdb
{
    class AvrGdbRsp: public GdbRspDebugServer<AvrGdbTargetDescriptor, Gdb::DebugSession, CommandPackets::CommandPacket>
    {
    public:
        AvrGdbRsp(
            const DebugServerConfig& debugServerConfig,
            const Targets::TargetDescriptor& targetDescriptor,
            EventListener& eventListener,
            EventFdNotifier& eventNotifier
        );

        std::string getName() const override {
            return "AVR GDB Remote Serial Protocol Debug Server";
        }

    protected:
        std::unique_ptr<CommandPackets::CommandPacket> rawPacketToCommandPacket(const RawPacket& rawPacket) override;
        std::set<std::pair<Feature, std::optional<std::string>>> getSupportedFeatures() override;
        void handleTargetStoppedGdbResponse(Targets::TargetMemoryAddress programAddress) override;
    };
}
