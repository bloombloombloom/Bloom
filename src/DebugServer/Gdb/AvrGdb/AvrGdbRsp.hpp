#pragma once

#include <cstdint>

#include "src/DebugServer/Gdb/GdbRspDebugServer.hpp"

#include "AvrGdbTargetDescriptor.hpp"
#include "CommandPackets/CommandPacket.hpp"

namespace DebugServer::Gdb::AvrGdb
{
    class AvrGdbRsp: public GdbRspDebugServer<AvrGdbTargetDescriptor, DebugSession, CommandPackets::CommandPacket>
    {
    public:
        AvrGdbRsp(
            const DebugServerConfig& debugServerConfig,
            const Targets::TargetDescriptor& targetDescriptor,
            EventListener& eventListener,
            EventFdNotifier& eventNotifier
        );

        std::string getName() const override;

    protected:
        std::unique_ptr<CommandPackets::CommandPacket> rawPacketToCommandPacket(const RawPacket& rawPacket) override;
        std::set<std::pair<Feature, std::optional<std::string>>> getSupportedFeatures() override;
        void handleTargetStoppedGdbResponse(Targets::TargetMemoryAddress programAddress) override;
    };
}
