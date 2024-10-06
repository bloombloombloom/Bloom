#include "TargetMemoryCache.hpp"

#include <algorithm>

#include "src/Exceptions/Exception.hpp"

namespace Targets
{
    TargetMemoryCache::TargetMemoryCache(const TargetMemorySegmentDescriptor& memorySegmentDescriptor)
        : memorySegmentDescriptor(memorySegmentDescriptor)
        , data(TargetMemoryBuffer(memorySegmentDescriptor.size(), 0x00))
    {}

    TargetMemoryBuffer TargetMemoryCache::fetch(TargetMemoryAddress startAddress, TargetMemorySize bytes) const {
        const auto startIndex = startAddress - this->memorySegmentDescriptor.addressRange.startAddress;

        if (
            startAddress < this->memorySegmentDescriptor.addressRange.startAddress
            || (startIndex + bytes) > this->data.size()
        ) {
            throw Exceptions::Exception{"Invalid cache access"};
        }

        return TargetMemoryBuffer{this->data.begin() + startIndex, this->data.begin() + startIndex + bytes};
    }

    bool TargetMemoryCache::contains(TargetMemoryAddress startAddress, TargetMemorySize bytes) const {
        const auto intersectingSegmentIt = this->intersectingSegment(startAddress);

        return
            intersectingSegmentIt != this->populatedSegments.end()
            && intersectingSegmentIt->first <= startAddress
            && intersectingSegmentIt->second >= (startAddress + bytes - 1);
    }

    void TargetMemoryCache::insert(TargetMemoryAddress startAddress, const TargetMemoryBuffer& data) {
        const auto startIndex = startAddress - this->memorySegmentDescriptor.addressRange.startAddress;

        std::copy(data.begin(), data.end(), this->data.begin() + startIndex);

        const auto endAddress = static_cast<Targets::TargetMemoryAddress>(startAddress + data.size() - 1);

        const auto intersectingStartSegmentIt = this->intersectingSegment(startAddress);
        const auto intersectingEndSegmentIt = this->intersectingSegment(endAddress);

        if (
            intersectingStartSegmentIt == this->populatedSegments.end()
            && intersectingEndSegmentIt == this->populatedSegments.end()
        ) {
            this->populatedSegments.emplace(startAddress, endAddress);
            return;
        }

        if (intersectingStartSegmentIt == intersectingEndSegmentIt) {
            // We already have a populated segment containing this address range. Nothing to do here.
            return;
        }

        auto newStartSegment = std::optional<decltype(this->populatedSegments)::value_type>{};
        auto newEndSegment = std::optional<decltype(this->populatedSegments)::value_type>{};

        if (intersectingStartSegmentIt != this->populatedSegments.end()) {
            newStartSegment.emplace(intersectingStartSegmentIt->first, endAddress);
            this->populatedSegments.erase(intersectingStartSegmentIt);
        }

        if (intersectingEndSegmentIt != this->populatedSegments.end()) {
            newEndSegment.emplace(startAddress, intersectingEndSegmentIt->second);
            this->populatedSegments.erase(intersectingEndSegmentIt);
        }

        if (newStartSegment.has_value() && newEndSegment.has_value()) {
            // The two new segments overlap. Merge them into one and remove any segments between them
            newStartSegment.emplace(newStartSegment->first, newEndSegment->second);
            newEndSegment.reset();

            for (
                auto segmentIt = this->populatedSegments.upper_bound(newStartSegment->first);
                segmentIt != this->populatedSegments.end();
            ) {
                if (segmentIt->second > newStartSegment->second) {
                    break;
                }

                this->populatedSegments.erase(segmentIt++);
            }
        }

        if (newStartSegment.has_value()) {
            this->populatedSegments.insert(*newStartSegment);
        }

        if (newEndSegment.has_value()) {
            this->populatedSegments.insert(*newEndSegment);
        }
    }

    void TargetMemoryCache::clear() {
        this->populatedSegments.clear();
    }

    TargetMemoryCache::SegmentIt TargetMemoryCache::intersectingSegment(TargetMemoryAddress address) const {
        if (this->populatedSegments.empty()) {
            return this->populatedSegments.end();
        }

        auto lowerBoundSegmentIt = this->populatedSegments.lower_bound(address);

        if (lowerBoundSegmentIt != this->populatedSegments.end()) {
            if (lowerBoundSegmentIt->first != address && lowerBoundSegmentIt != this->populatedSegments.begin()) {
                --lowerBoundSegmentIt;
            }

            return lowerBoundSegmentIt->first <= address && lowerBoundSegmentIt->second >= address
                ? lowerBoundSegmentIt
                : this->populatedSegments.end();
        }

        // All the segments precede the given address. If the last segment doesn't intersect, none of them will.
        const auto lastSegment = std::prev(this->populatedSegments.end());
        return lastSegment->first <= address && lastSegment->second >= address
            ? lastSegment
            : this->populatedSegments.end();
    }
}
