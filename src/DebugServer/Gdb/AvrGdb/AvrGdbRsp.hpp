#pragma once

#include <cstdint>

#include "TargetDescriptor.hpp"

#include "src/DebugServer/Gdb/GdbRspDebugServer.hpp"

namespace Bloom::DebugServer::Gdb::AvrGdb
{
    class AvrGdbRsp: public GdbRspDebugServer
    {
    public:
        AvrGdbRsp(
            const DebugServerConfig& debugServerConfig,
            EventListener& eventListener,
            EventFdNotifier& eventNotifier
        );

        std::string getName() const override {
            return "AVR GDB Remote Serial Protocol Debug Server";
        }

    protected:
        void init() override;

        const Gdb::TargetDescriptor& getGdbTargetDescriptor() override {
            return this->gdbTargetDescriptor.value();
        }

        std::unique_ptr<Gdb::CommandPackets::CommandPacket> resolveCommandPacket(
            const RawPacketType& rawPacket
        ) override;

        std::set<std::pair<Feature, std::optional<std::string>>> getSupportedFeatures() override ;

    private:
        std::optional<TargetDescriptor> gdbTargetDescriptor;
    };
}
