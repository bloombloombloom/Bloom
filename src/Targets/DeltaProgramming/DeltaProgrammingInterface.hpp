#pragma once

#include "Session.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"

namespace Targets::DeltaProgramming
{
    class DeltaProgrammingInterface
    {
    public:
        DeltaProgrammingInterface() = default;
        virtual ~DeltaProgrammingInterface() = default;

        /**
         * The TargetController can align delta segments to a given block size, using the program memory cache.
         * This can significantly reduce the number of read operations that would take place for page alignment,
         * improving programming speed.
         *
         * This member function should return the necessary block size for the given memory segment. If alignment is
         * not required, it should return a value of 1.
         */
        virtual TargetMemorySize deltaBlockSize(
            const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const TargetMemorySegmentDescriptor& memorySegmentDescriptor
        ) = 0;

        virtual bool shouldAbandonSession(
            const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            const std::vector<Session::WriteOperation::Region>& deltaSegments
        ) = 0;
    };
}
