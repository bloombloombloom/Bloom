#pragma once

#include <cstdint>

#include "src/DebugServer/Gdb/GdbRspDebugServer.hpp"
#include "RiscVGdbTargetDescriptor.hpp"
#include "CommandPackets/RiscVGdbCommandPacketInterface.hpp"

namespace DebugServer::Gdb::RiscVGdb
{
    class RiscVGdbRsp
        : public GdbRspDebugServer<
            RiscVGdbTargetDescriptor,
            DebugSession,
            CommandPackets::RiscVGdbCommandPacketInterface
        >
    {
    public:
        RiscVGdbRsp(
            const DebugServerConfig& debugServerConfig,
            const Targets::TargetDescriptor& targetDescriptor,
            EventListener& eventListener,
            EventFdNotifier& eventNotifier
        );

        std::string getName() const override;

    protected:
        std::unique_ptr<CommandPackets::RiscVGdbCommandPacketInterface> rawPacketToCommandPacket(
            const RawPacket& rawPacket
        ) override;
        std::set<std::pair<Feature, std::optional<std::string>>> getSupportedFeatures() override;
        void handleTargetStoppedGdbResponse(Targets::TargetMemoryAddress programAddress) override;
    };
}
