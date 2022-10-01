#pragma once

#include <cstdint>
#include <optional>

#include "CommandPacket.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    /**
     * The StepExecution class implements the structure for "s" command packets. Upon receiving this command, the
     * server is expected to step execution on the target.
     */
    class StepExecution: public CommandPacket
    {
    public:
        /**
         * The address from which to begin the step.
         */
        std::optional<Targets::TargetProgramCounter> fromProgramCounter;

        explicit StepExecution(const RawPacket& rawPacket);

        void handle(
            DebugSession& debugSession,
            TargetController::TargetControllerConsole& targetControllerConsole
        ) override;
    };
}
