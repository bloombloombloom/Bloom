#pragma once

#include <set>

#include "src/Targets/TargetMemory.hpp"

namespace DebugServer::Gdb
{
    /**
     * A range stepping session is created upon the receipt of a VContRangeStep command, from GDB.
     *
     * Any information related to the range stepping session is held here.
     *
     * See DebugSession::activeRangeSteppingSession for more.
     */
    struct RangeSteppingSession
    {
        /**
         * The (byte) address range we're stepping over.
         *
         * NOTE: range::endAddress is exclusive!
         */
        Targets::TargetMemoryAddressRange range;

        /**
         * Any program memory (byte) addresses that we had to intercept as part of this session.
         */
        std::set<Targets::TargetMemoryAddress> interceptedAddresses;

        /**
         * Whether we're currently performing a single step, in this session, to observe the behaviour of a particular
         * instruction.
         *
         * See AvrGdbRsp::handleTargetStoppedGdbResponse() for more.
         */
        bool singleStepping = false;

        RangeSteppingSession(
            const Targets::TargetMemoryAddressRange& range,
            const std::set<Targets::TargetMemoryAddress>& interceptedAddresses
        )
            : range(range)
            , interceptedAddresses(interceptedAddresses)
        {};
    };
}
