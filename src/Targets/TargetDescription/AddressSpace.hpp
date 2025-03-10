#pragma once

#include <string>
#include <cstdint>
#include <optional>
#include <map>
#include <utility>

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
        std::uint8_t unitSize;
        std::optional<TargetMemoryEndianness> endianness;
        std::map<std::string, MemorySegment, std::less<void>> memorySegmentsByKey;

        AddressSpace(
            const std::string& key,
            TargetMemoryAddress startAddress,
            TargetMemorySize size,
            std::uint8_t unitSize,
            const std::optional<TargetMemoryEndianness>& endianness,
            std::map<std::string, MemorySegment, std::less<void>>&& memorySegmentsByKey
        )
            : key(key)
            , startAddress(startAddress)
            , size(size)
            , unitSize(unitSize)
            , endianness(endianness)
            , memorySegmentsByKey(std::move(memorySegmentsByKey))
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
                throw Exceptions::InvalidTargetDescriptionDataException{
                    "Failed to get memory segment \"" + std::string{key}
                        + "\" from address space in TDF - segment not found"
                };
            }

            return segment->get();
        }
    };
}
