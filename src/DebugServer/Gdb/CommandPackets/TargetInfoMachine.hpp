#pragma once

#include <cstdint>
#include <QJsonObject>

#include "Monitor.hpp"

#include "src/Targets/TargetDescriptor.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    /**
     * The TargetInfoMachine class implements a structure for the "monitor target-info machine" GDB command.
     *
     * We just output information on the connected target, in JSON format.
     */
    class TargetInfoMachine: public Monitor
    {
    public:
        explicit TargetInfoMachine(Monitor&& monitorPacket);

        void handle(
            DebugSession& debugSession,
            TargetController::TargetControllerConsole& targetControllerConsole
        ) override;

    private:
        QJsonObject generateTargetInfo(const Targets::TargetDescriptor& targetDescriptor) const;
    };
}
