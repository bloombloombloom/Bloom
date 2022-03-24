#pragma once

#include "CommandPacket.hpp"

namespace Bloom::DebugServers::Gdb::CommandPackets
{
    /**
     * The InterruptException class represents interrupt command packets. Upon receiving an interrupt packet, the
     * server is expected to interrupt execution on the target.
     *
     * Technically, interrupts are not sent by the client in the form of a typical GDP RSP packet. Instead, they're
     * just sent as a single byte from the client. We fake the packet on our end, to save us the headache of dealing
     * with this inconsistency. We do this in CommandPacketFactory::extractRawPackets().
     */
    class InterruptExecution: public CommandPacket
    {
    public:
        explicit InterruptExecution(const std::vector<unsigned char>& rawPacket): CommandPacket(rawPacket) {}

        void handle(DebugSession& debugSession, TargetControllerConsole& targetControllerConsole) override;
    };
}
