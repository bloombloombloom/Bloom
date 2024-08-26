#pragma once

#include <cstdint>

#include "Monitor.hpp"

#include "src/Targets/TargetRegisterDescriptor.hpp"
#include "src/Targets/TargetRegisterGroupDescriptor.hpp"
#include "src/Targets/TargetPeripheralDescriptor.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    class ReadRegistersMonitor: public Monitor
    {
    public:
        explicit ReadRegistersMonitor(Monitor&& monitorPacket);

        void handle(
            DebugSession& debugSession,
            const TargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;

    protected:
        void handleSingleRegisterOutput(
            const Targets::TargetRegisterDescriptor& registerDescriptor,
            DebugSession& debugSession,
            Services::TargetControllerService& targetControllerService
        );

        void handlePeripheralOutput(
            const Targets::TargetPeripheralDescriptor& peripheralDescriptor,
            DebugSession& debugSession,
            Services::TargetControllerService& targetControllerService
        );

        void handleRegisterGroupOutput(
            const Targets::TargetRegisterGroupDescriptor& groupDescriptor,
            DebugSession& debugSession,
            Services::TargetControllerService& targetControllerService
        );
    };
}
