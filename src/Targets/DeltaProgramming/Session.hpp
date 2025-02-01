#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"
#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetMemoryAddressRange.hpp"

namespace Targets::DeltaProgramming
{
    struct Session
    {
        struct EraseOperation
        {
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor;
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor;
        };

        struct WriteOperation
        {
            struct Region
            {
                Targets::TargetMemoryAddressRange addressRange;
                Targets::TargetMemoryBuffer buffer;

                void mergeWith(const Region& other);
            };

            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor;
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor;
            std::vector<Region> regions;

            [[nodiscard]] std::vector<Region> deltaSegments(
                Targets::TargetMemoryBufferSpan cacheData,
                Targets::TargetMemorySize blockSize
            ) const;
        };

        std::unordered_map<Targets::TargetMemorySegmentId, EraseOperation> eraseOperationsBySegmentId;
        std::unordered_map<Targets::TargetMemorySegmentId, WriteOperation> writeOperationsBySegmentId;

        void pushEraseOperation(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor
        );

        void pushWriteOperation(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemoryBuffer&& buffer
        );
    };
}
