#pragma once

#include <optional>

#include "CommandPacket.hpp"

namespace Bloom::DebugServers::Gdb::CommandPackets
{
    /**
     * The StepExecution class implements the structure for "s" command packets. Upon receiving this command, the
     * server is expected to step execution on the target.
     */
    class StepExecution: public CommandPacket
    {
    private:
        void init();

    public:
        /**
         * The address from which to begin the step.
         */
        std::optional<size_t> fromProgramCounter;

        explicit StepExecution(const std::vector<unsigned char>& rawPacket): CommandPacket(rawPacket) {
            init();
        };

        void dispatchToHandler(Gdb::GdbRspDebugServer& gdbRspDebugServer) override;
    };
}
