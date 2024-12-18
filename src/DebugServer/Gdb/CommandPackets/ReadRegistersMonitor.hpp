#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <string>

#include "Monitor.hpp"

#include "src/Targets/TargetPeripheralDescriptor.hpp"
#include "src/Targets/TargetRegisterGroupDescriptor.hpp"
#include "src/Targets/TargetRegisterDescriptor.hpp"
#include "src/Targets/TargetBitFieldDescriptor.hpp"

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

        static std::vector<const Targets::TargetRegisterDescriptor*> sortRegisterDescriptors(
            const std::map<std::string, Targets::TargetRegisterDescriptor, std::less<void>>& map
        );

        static std::vector<const Targets::TargetBitFieldDescriptor*> sortBitFieldDescriptors(
            const std::map<std::string, Targets::TargetBitFieldDescriptor>& map
        );
    };
}
