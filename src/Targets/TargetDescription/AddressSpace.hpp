#pragma once

#include <string>
#include <optional>
#include <map>

#include "MemorySegment.hpp"
#include "src/Targets/TargetMemory.hpp"

#include "Exceptions/InvalidTargetDescriptionDataException.hpp"

namespace Targets::TargetDescription
{
    struct AddressSpace
    {
        std::string key;
        TargetMemoryAddress startAddress;
        TargetMemorySize size;
        std::optional<TargetMemoryEndianness> endianness;
        std::map<std::string, MemorySegment, std::less<void>> memorySegmentsByKey;

        AddressSpace(
            const std::string& key,
            TargetMemoryAddress startAddress,
            TargetMemorySize size,
            const std::optional<TargetMemoryEndianness>& endianness,
            const std::map<std::string, MemorySegment, std::less<void>>& memorySegmentsByKey
        )
            : key(key)
            , startAddress(startAddress)
            , size(size)
            , endianness(endianness)
            , memorySegmentsByKey(memorySegmentsByKey)
        {}

        std::optional<std::reference_wrapper<const MemorySegment>> tryGetMemorySegment(std::string_view key) const {
            const auto segmentIt = this->memorySegmentsByKey.find(key);

            if (segmentIt == this->memorySegmentsByKey.end()) {
                return std::nullopt;
            }

            return std::cref(segmentIt->second);
        }

        const MemorySegment& getMemorySegment(std::string_view key) const {
            const auto segment = this->tryGetMemorySegment(key);
            if (!segment.has_value()) {
                throw Exceptions::InvalidTargetDescriptionDataException(
                    "Failed to get memory segment \"" + std::string(key)
                        + "\" from address space in TDF - segment not found"
                );
            }

            return segment->get();
        }
    };
}
