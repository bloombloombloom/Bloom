#pragma once

#include <cstdint>
#include <optional>

#include "CommandPacket.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    /**
     * The ContinueExecution class implements a structure for "c" packets. These packets instruct the server
     * to continue execution on the target.
     *
     * See @link https://sourceware.org/gdb/onlinedocs/gdb/Packets.html#Packets for more on this.
     */
    class ContinueExecution: public CommandPacket
    {
    public:
        /**
         * The "c" packet can contain an address which specifies the point from which the execution should be resumed on
         * the target.
         *
         * Although the packet *can* contain this address, it is not required, hence the std::optional type.
         */
        std::optional<Targets::TargetMemoryAddress> fromAddress;

        explicit ContinueExecution(const RawPacket& rawPacket);

        bool requiresBreakpointFlush() const override {
            return true;
        }

        void handle(
            DebugSession& debugSession,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
