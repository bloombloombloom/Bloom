#pragma once

#include <vector>
#include <memory>

// Command packets
#include "CommandPacket.hpp"
#include "InterruptExecution.hpp"
#include "SupportedFeaturesQuery.hpp"
#include "ReadGeneralRegisters.hpp"
#include "WriteGeneralRegister.hpp"
#include "ReadMemory.hpp"
#include "WriteMemory.hpp"
#include "StepExecution.hpp"
#include "ContinueExecution.hpp"
#include "SetBreakpoint.hpp"
#include "RemoveBreakpoint.hpp"

namespace Bloom::DebugServers::Gdb
{
    /**
     * The CommandPacketFactory class provides a means for extracting raw packet data from a raw buffer, and
     * constructing the appropriate CommandPacket objects.
     */
    class CommandPacketFactory
    {
    public:
        /**
         * Extracts raw GDB RSP packets from buffer.
         *
         * @param buffer
         *  The buffer from which to extract the raw GDB RSP packets.
         *
         * @return
         *  A vector of raw packets.
         */
        static std::vector<std::vector<unsigned char>> extractRawPackets(std::vector<unsigned char> buffer);

        /**
         * Constructs the appropriate CommandPacket object from a single raw GDB RSP packet.
         *
         * @param rawPacket
         *  The raw GDB RSP packet from which to construct the CommandPacket object.
         *
         * @return
         */
        static std::unique_ptr<CommandPackets::CommandPacket> create(std::vector<unsigned char> rawPacket);
    };
}
