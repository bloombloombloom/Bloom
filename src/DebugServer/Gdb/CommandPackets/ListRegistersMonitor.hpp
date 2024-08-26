#pragma once

#include <cstdint>

#include "Monitor.hpp"

#include "src/Targets/TargetRegisterDescriptor.hpp"
#include "src/Targets/TargetRegisterGroupDescriptor.hpp"
#include "src/Targets/TargetPeripheralDescriptor.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    class ListRegistersMonitor: public Monitor
    {
    public:
        explicit ListRegistersMonitor(Monitor&& monitorPacket);

        void handle(
            DebugSession& debugSession,
            const TargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;

    protected:
        void handlePeripheralOutput(
            const Targets::TargetPeripheralDescriptor& peripheralDescriptor,
            DebugSession& debugSession
        );

        void handleRegisterGroupOutput(
            const Targets::TargetRegisterGroupDescriptor& groupDescriptor,
            DebugSession& debugSession
        );
    };
}
