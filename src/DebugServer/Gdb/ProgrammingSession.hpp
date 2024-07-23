#pragma once

#include "src/Targets/TargetMemory.hpp"

namespace DebugServer::Gdb
{
    /**
     * A programming session is created upon receiving the first FlashWrite (vFlashWrite) packet from GDB.
     *
     * The programming session holds the start address and a single buffer, which contains the sum of the numerous
     * buffers received by GDB (via multiple FlashWrite packets). Upon receiving a FlashDone (vFlashDone) packet, we
     * write the whole buffer to the target's program memory and then destroy the programming session.
     *
     * See FlashWrite::handle() and FlashDone::handle() for more.
     */
    struct ProgrammingSession
    {
        Targets::TargetMemoryAddress startAddress;
        Targets::TargetMemoryBuffer buffer;

        ProgrammingSession(
            Targets::TargetMemoryAddress startAddress,
            const Targets::TargetMemoryBuffer& initialBuffer
        )
            : startAddress(startAddress)
            , buffer(initialBuffer)
        {};
    };
}
