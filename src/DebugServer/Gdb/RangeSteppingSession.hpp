#pragma once

#include <set>

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"

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
         * The address space and memory segment of the program memory which we are stepping over.
         */
        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor;
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor;

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
         * Whether we're currently performing a single step, in this session, to start the session or observe the
         * behaviour of a particular instruction.
         *
         * See AvrGdbRsp::handleTargetStoppedGdbResponse() for more.
         */
        bool singleStepping = false;

        RangeSteppingSession(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            const Targets::TargetMemoryAddressRange& range,
            const std::set<Targets::TargetMemoryAddress>& interceptedAddresses
        )
            : addressSpaceDescriptor(addressSpaceDescriptor)
            , memorySegmentDescriptor(memorySegmentDescriptor)
            , range(range)
            , interceptedAddresses(interceptedAddresses)
        {};

        RangeSteppingSession(RangeSteppingSession&& other) noexcept
            : addressSpaceDescriptor(other.addressSpaceDescriptor)
            , memorySegmentDescriptor(other.memorySegmentDescriptor)
            , range(other.range)
            , interceptedAddresses(std::move(other.interceptedAddresses))
            , singleStepping(other.singleStepping)
        {}
    };
}
