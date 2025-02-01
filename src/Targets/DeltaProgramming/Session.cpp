#include "Session.hpp"

#include <algorithm>
#include <cassert>

#include "src/Services/AlignmentService.hpp"

namespace Targets::DeltaProgramming
{
    using Targets::TargetAddressSpaceDescriptor;
    using Targets::TargetMemorySegmentDescriptor;
    using Targets::TargetMemoryAddress;
    using Targets::TargetMemoryAddressRange;
    using Targets::TargetMemoryBuffer;

    void Session::WriteOperation::Region::mergeWith(const Region& other) {
        assert(this->addressRange.intersectsWith(other.addressRange));
        assert(this->addressRange.startAddress <= other.addressRange.startAddress);

        const auto intersectingSize = other.addressRange.intersectingSize(this->addressRange);
        if (intersectingSize < addressRange.size()) {
            this->buffer.resize(this->buffer.size() + other.buffer.size() - intersectingSize);
            this->addressRange.endAddress = static_cast<Targets::TargetMemoryAddress>(
                this->addressRange.startAddress + this->buffer.size() - 1
            );
        }

        std::copy(
            other.buffer.begin(),
            other.buffer.end(),
            this->buffer.begin() + (other.addressRange.startAddress - this->addressRange.startAddress)
        );
    }

    std::vector<Session::WriteOperation::Region> Session::WriteOperation::deltaSegments(
        Targets::TargetMemoryBufferSpan cacheData,
        Targets::TargetMemorySize blockSize
    ) const {
        using Services::AlignmentService;

        // First, we merge any overlapping regions and sort all regions by start address. This simplifies things
        auto mergedRegions = std::vector<Region>{};

        for (const auto& region : this->regions) {
            for (auto& existingRegion : mergedRegions) {
                if (!existingRegion.addressRange.intersectsWith(region.addressRange)) {
                    continue;
                }

                if (region.addressRange.startAddress >= existingRegion.addressRange.startAddress) {
                    existingRegion.mergeWith(region);

                } else {
                    auto regionClone = region;
                    regionClone.mergeWith(existingRegion);
                    std::swap(existingRegion, regionClone);
                }

                goto CONTINUE_OUTER;
            }

            mergedRegions.emplace_back(region);

            CONTINUE_OUTER:
            continue;
        }

        std::sort(
            mergedRegions.begin(),
            mergedRegions.end(),
            [] (const Region& regionA, const Region& regionB) {
                return regionA.addressRange.startAddress < regionB.addressRange.startAddress;
            }
        );

        auto output = std::vector<Region>{};
        auto deltaSegment = std::optional<Region>{};

        /*
         * Given that delta segments must be aligned to a given block size, not all bytes in a delta segment will
         * differ from the corresponding bytes in the cached data.
         *
         * We enforce alignment of all delta segments from the point of creation, and maintain alignment during any
         * subsequent changes. This simplifies things.
         *
         * We use the cached data to align the segments. Initially, we populate the aligned segments with the cached
         * data, and then gradually overwrite the bytes that differ.
         */
        for (const auto& region : mergedRegions) {
            for (auto i = std::size_t{0}; i < region.buffer.size(); ++i) {
                const auto address = static_cast<TargetMemoryAddress>(region.addressRange.startAddress + i);
                const auto cacheIndex = static_cast<std::size_t>(
                    address - this->memorySegmentDescriptor.addressRange.startAddress
                );
                const auto regionByte = region.buffer[i];
                const auto cachedByte = cacheData[cacheIndex];

                if (regionByte == cachedByte) {
                    continue;
                }

                if (deltaSegment.has_value() && address > (deltaSegment->addressRange.endAddress + blockSize)) {
                    /*
                     * This region byte is not within the boundary of the current delta segment, or in the neighbouring
                     * block, so commit and create a new one.
                     */
                    output.emplace_back(std::move(*deltaSegment));
                    deltaSegment = std::nullopt;
                }

                if (!deltaSegment.has_value()) {
                    const auto alignedAddress = AlignmentService::alignMemoryAddress(address, blockSize);
                    const auto alignedSize = AlignmentService::alignMemorySize((address - alignedAddress) + 1, blockSize);

                    const auto cacheOffset = cacheData.begin() + static_cast<long>(
                        alignedAddress - this->memorySegmentDescriptor.addressRange.startAddress
                    );
                    deltaSegment = Region{
                        .addressRange = TargetMemoryAddressRange{alignedAddress, alignedAddress + alignedSize - 1},
                        .buffer = {cacheOffset, cacheOffset + alignedSize}
                    };
                }

                if (address > deltaSegment->addressRange.endAddress) {
                    /*
                     * This region byte is in the neighbouring block of the current delta segment.
                     *
                     * Instead of committing the segment and creating a new one, we just extend it by another block,
                     * to accommodate the byte. This reduces the number of segments (and therefore, the number of
                     * memory writes).
                     */
                    const auto cacheOffset = cacheData.begin() + static_cast<long>(
                        (deltaSegment->addressRange.endAddress
                            - this->memorySegmentDescriptor.addressRange.startAddress) + 1
                    );
                    deltaSegment->buffer.insert(deltaSegment->buffer.end(), cacheOffset, cacheOffset + blockSize);
                    deltaSegment->addressRange.endAddress += blockSize;
                }

                deltaSegment->buffer[address - deltaSegment->addressRange.startAddress] = regionByte;
            }
        }

        if (deltaSegment.has_value()) {
            output.emplace_back(std::move(*deltaSegment));
        }

        return output;
    }

    void Session::pushEraseOperation(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor
    ) {
        if (this->eraseOperationsBySegmentId.contains(memorySegmentDescriptor.id)) {
            return;
        }

        this->eraseOperationsBySegmentId.emplace(
            memorySegmentDescriptor.id,
            EraseOperation{
                .addressSpaceDescriptor = addressSpaceDescriptor,
                .memorySegmentDescriptor = memorySegmentDescriptor,
            }
        );
    }

    void Session::pushWriteOperation(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        TargetMemoryAddress startAddress,
        Targets::TargetMemoryBuffer&& buffer
    ) {
        assert(!buffer.empty());

        auto operationIt = this->writeOperationsBySegmentId.find(memorySegmentDescriptor.id);
        if (operationIt == this->writeOperationsBySegmentId.end()) {
            operationIt = this->writeOperationsBySegmentId.emplace(
                memorySegmentDescriptor.id,
                WriteOperation{
                    .addressSpaceDescriptor = addressSpaceDescriptor,
                    .memorySegmentDescriptor = memorySegmentDescriptor,
                    .regions = {},
                }
            ).first;
        }

        operationIt->second.regions.emplace_back(
            WriteOperation::Region{
                .addressRange = TargetMemoryAddressRange{
                    startAddress,
                    static_cast<TargetMemoryAddress>(startAddress + buffer.size() - 1)
                },
                .buffer = std::move(buffer)
            }
        );
    }
}
