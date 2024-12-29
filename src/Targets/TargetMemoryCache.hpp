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
        explicit TargetMemoryCache(const TargetMemorySegmentDescriptor& memorySegmentDescriptor);

        /**
         * Fetches data from the cache.
         *
         * TODO: Change return type to TargetMemoryBufferSpan
         *
         * @param startAddress
         * @param bytes
         *
         * @return
         */
        [[nodiscard]] TargetMemoryBuffer fetch(TargetMemoryAddress startAddress, TargetMemorySize bytes) const;

        /**
         * Checks if the cache currently holds data within the given address range.
         *
         * @param startAddress
         * @param bytes
         *
         * @return
         */
        [[nodiscard]] bool contains(TargetMemoryAddress startAddress, TargetMemorySize bytes) const;

        /**
         * Inserts data into the cache and performs any necessary bookkeeping.
         *
         * @param startAddress
         * @param data
         */
        void insert(TargetMemoryAddress startAddress, TargetMemoryBufferSpan data);

        /**
         * Clears the cache.
         */
        void clear();

    private:
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor;
        TargetMemoryBuffer data;

        /**
         * A populated segment is just an address range in the cache that we know we've populated.
         *
         * populatedSegments::value_type::first = The start address (inclusive) of the populated range
         * populatedSegments::value_type::second = The end address (inclusive) of the populated range
         */
        std::map<TargetMemoryAddress, TargetMemoryAddress> populatedSegments = {};

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
