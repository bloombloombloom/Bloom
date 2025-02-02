#pragma once

#include "CommandPacket.hpp"

#include "src/ProjectConfig.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    class Detach: public CommandPacket
    {
    public:
        explicit Detach(const RawPacket& rawPacket, const EnvironmentConfig& environmentConfig);

        void handle(
            DebugSession& debugSession,
            const TargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;

    private:
        const EnvironmentConfig& environmentConfig;
    };
}
