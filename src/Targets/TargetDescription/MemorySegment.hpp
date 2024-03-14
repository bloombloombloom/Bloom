#pragma once

#include <cstdint>
#include <optional>

#include "src/Targets/TargetMemorySegmentType.hpp"

#include "MemorySegmentSection.hpp"

#include "src/Services/StringService.hpp"
#include "Exceptions/InvalidTargetDescriptionDataException.hpp"

namespace Targets::TargetDescription
{
    struct MemorySegment
    {
        std::string key;
        std::string name;
        TargetMemorySegmentType type;
        std::uint32_t startAddress;
        std::uint32_t size;
        std::optional<std::uint16_t> pageSize;
        std::map<std::string, MemorySegmentSection, std::less<void>> sectionsByKey;

        MemorySegment(
            const std::string& key,
            const std::string& name,
            TargetMemorySegmentType type,
            std::uint32_t startAddress,
            std::uint32_t size,
            const std::optional<std::uint16_t>& pageSize,
            const std::map<std::string, MemorySegmentSection, std::less<void>>& sectionsByKey
        )
            : key(key)
            , name(name)
            , type(type)
            , startAddress(startAddress)
            , size(size)
            , pageSize(pageSize)
            , sectionsByKey(sectionsByKey)
        {}

        std::optional<std::reference_wrapper<const MemorySegmentSection>> tryGetSection(
            std::string_view keyStr
        ) const {
            const auto keys = Services::StringService::split(keyStr, '.');

            const auto firstSubgroupIt = this->sectionsByKey.find(*keys.begin());
            return firstSubgroupIt != this->sectionsByKey.end()
                ? keys.size() > 1
                    ? firstSubgroupIt->second.tryGetSubSection(keys | std::ranges::views::drop(1))
                    : std::optional(std::cref(firstSubgroupIt->second))
                : std::nullopt;
        }

        const MemorySegmentSection& getSection(std::string_view keyStr) const {
            const auto propertyGroup = this->tryGetSection(keyStr);
            if (!propertyGroup.has_value()) {
                throw Exceptions::InvalidTargetDescriptionDataException(
                    "Failed to get memory segment section \"" + std::string(keyStr)
                        + "\" from memory segment in TDF - section not found"
                );
            }

            return propertyGroup->get();
        }
    };
}
