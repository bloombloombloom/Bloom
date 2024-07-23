#pragma once

#include <string>
#include <map>
#include <optional>
#include <concepts>
#include <functional>
#include <ranges>

#include "src/Targets/TargetMemory.hpp"

#include "src/Services/StringService.hpp"
#include "Exceptions/InvalidTargetDescriptionDataException.hpp"

namespace Targets::TargetDescription
{
    struct MemorySegmentSection
    {
        std::string key;
        std::string name;
        TargetMemoryAddress startAddress;
        TargetMemorySize size;
        std::map<std::string, MemorySegmentSection, std::less<void>> subSectionsByKey;

        MemorySegmentSection(
            const std::string& key,
            const std::string& name,
            TargetMemoryAddress startAddress,
            TargetMemorySize size,
            const std::map<std::string, MemorySegmentSection, std::less<void>>& subSectionsByKey
        )
            : key(key)
            , name(name)
            , startAddress(startAddress)
            , size(size)
            , subSectionsByKey(subSectionsByKey)
        {}

        template <typename KeysType>
        requires
            std::ranges::sized_range<KeysType>
        std::optional<std::reference_wrapper<const MemorySegmentSection>> tryGetSubSection(KeysType keys) const {
            const auto firstSubSectionIt = this->subSectionsByKey.find(*(keys.begin()));
            if (firstSubSectionIt == this->subSectionsByKey.end()) {
                return std::nullopt;
            }

            auto subSection = std::optional(std::cref(firstSubSectionIt->second));
            for (const auto key : keys | std::ranges::views::drop(1)) {
                subSection = subSection->get().tryGetSubSection(key);

                if (!subSection.has_value()) {
                    break;
                }
            }

            return subSection;
        }

        std::optional<std::reference_wrapper<const MemorySegmentSection>> tryGetSubSection(
            std::string_view keyStr
        ) const {
            return this->tryGetSubSection(Services::StringService::split(keyStr, '.'));
        }

        const MemorySegmentSection& getSubSection(std::string_view keyStr) const {
            const auto propertyGroup = this->tryGetSubSection(keyStr);
            if (!propertyGroup.has_value()) {
                throw Exceptions::InvalidTargetDescriptionDataException{
                    "Failed to get memory segment sub-section \"" + std::string{keyStr}
                        + "\" from memory segment in TDF - sub-section not found"
                };
            }

            return propertyGroup->get();
        }
    };
}
