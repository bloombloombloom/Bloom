#pragma once

#include <cstdint>

#include "Monitor.hpp"

#include "src/Targets/TargetDescriptor.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    /**
     * The GenerateSvd class implements a structure for the "monitor svd" GDB command.
     *
     * This command generates XML conforming to the CMSIS-SVD schema, for the connected target. Will output the XML to
     * a file or send it to GDB.
     */
    class GenerateSvd: public Monitor
    {
    public:
        explicit GenerateSvd(Monitor&& monitorPacket);

        void handle(
            DebugSession& debugSession,
            TargetController::TargetControllerConsole& targetControllerConsole
        ) override;

    private:
        bool sendOutput = false;

        QDomDocument generateSvd(
            const Targets::TargetDescriptor& targetDescriptor,
            std::uint32_t baseAddressOffset = 0
        );
    };
}
