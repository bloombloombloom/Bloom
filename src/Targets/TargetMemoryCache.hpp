#pragma once

#include <cstdint>
#include <map>

#include "TargetMemorySegmentDescriptor.hpp"
#include "TargetMemory.hpp"

namespace Targets
{
    class TargetMemoryCache
    {
    public:
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor;
        TargetMemoryBuffer data;

        explicit TargetMemoryCache(const TargetMemorySegmentDescriptor& memorySegmentDescriptor);

        [[nodiscard]] TargetMemoryBufferSpan fetch(TargetMemoryAddress startAddress, TargetMemorySize bytes) const;
        [[nodiscard]] bool contains(TargetMemoryAddress startAddress, TargetMemorySize bytes) const;

        void insert(TargetMemoryAddress startAddress, TargetMemoryBufferSpan data);
        void fill(TargetMemoryAddress startAddress, TargetMemorySize size, unsigned char value);

        void clear();

    private:
        /**
         * A populated segment is just an address range in the cache that we know we've populated.
         *
         * populatedSegments::value_type::first = The start address (inclusive) of the populated range
         * populatedSegments::value_type::second = The end address (inclusive) of the populated range
         */
        std::map<TargetMemoryAddress, TargetMemoryAddress> populatedSegments = {};

        /**
         * Tracks a newly populated segment.
         *
         * @param startAddress
         * @param endAddress
         */
        void trackSegment(TargetMemoryAddress startAddress, TargetMemoryAddress endAddress);

        /**
         * Finds the segment that intersects with the given address. Segments cannot overlap, so only one segment can
         * intersect with the given address, at any given time.
         *
         * @param address
         *
         * @return
         *  An iterator to the intersecting segment, or populatedSegments::end() if none is found.
         */
        using SegmentIt = decltype(TargetMemoryCache::populatedSegments)::const_iterator;
        [[nodiscard]] SegmentIt intersectingSegment(TargetMemoryAddress address) const;
    };
}
