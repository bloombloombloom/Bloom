#pragma once

#include <cstdint>
#include <string>
#include <map>
#include <functional>
#include <optional>
#include <string_view>
#include <ranges>
#include <concepts>

#include "TargetMemory.hpp"
#include "TargetAddressSpaceDescriptor.hpp"
#include "TargetRegisterDescriptor.hpp"

namespace Targets
{
    struct TargetRegisterGroupDescriptor
    {
    public:
        std::string key;
        std::string name;
        std::string addressSpaceKey;
        const TargetAddressSpaceDescriptorId addressSpaceDescriptorId;
        std::optional<std::string> description;
        std::map<std::string, TargetRegisterDescriptor> registerDescriptorsByKey;
        std::map<std::string, TargetRegisterGroupDescriptor, std::less<void>> subgroupDescriptorsByKey;

        TargetRegisterGroupDescriptor(
            const std::string& key,
            const std::string& name,
            const std::string& addressSpaceKey,
            TargetAddressSpaceDescriptorId addressSpaceDescriptorId,
            const std::optional<std::string>& description,
            const std::map<std::string, TargetRegisterDescriptor>& registerDescriptorsByKey,
            const std::map<std::string, TargetRegisterGroupDescriptor, std::less<void>>& subgroupDescriptorsByKey
        );

        /**
         * Calculates the start address of the register group.
         *
         * Excessive calls to this function is discouraged, as the implementation is quite slow.
         *
         * @return
         */
        TargetMemoryAddress startAddress() const;

        /**
         * Calculates the size of this register group.
         *
         * Excessive calls to this function is discouraged, as the implementation is quite slow.
         *
         * @return
         */
        TargetMemorySize size() const;

        /**
         * Attempts to fetch a subgroup with a set of keys.
         *
         * @tparam KeysType
         * @param keys
         * @return
         */
        template <typename KeysType>
        requires
            std::ranges::sized_range<KeysType>
        std::optional<std::reference_wrapper<const TargetRegisterGroupDescriptor>> tryGetSubgroupDescriptor(
            KeysType keys
        ) const {
            auto firstSubgroupIt = this->subgroupDescriptorsByKey.find(*(keys.begin()));
            if (firstSubgroupIt == this->subgroupDescriptorsByKey.end()) {
                return std::nullopt;
            }

            auto subgroup = std::optional(std::cref(firstSubgroupIt->second));
            for (const auto key : keys | std::ranges::views::drop(1)) {
                subgroup = subgroup->get().tryGetSubgroupDescriptor(key);

                if (!subgroup.has_value()) {
                    break;
                }
            }

            return subgroup;
        }

        /**
         * Attempts to fetch a subgroup with a set of keys in the form of a string in "dot notation".
         *
         * @param keyStr
         *  A string in "dot notation", containing each key seperated with a period/dot character (e.g.
         *  "parent.child.grandchild").
         *
         * @return
         */
        std::optional<std::reference_wrapper<const TargetRegisterGroupDescriptor>> tryGetSubgroupDescriptor(
            std::string_view keyStr
        ) const;

        /**
         * Fetches a subgroup with a set of keys in the form of a string in "dot notation". If the subgroup is not
         * found, an InternalFatalErrorException is thrown.
         *
         * @param keyStr
         *  A string in "dot notation", containing each key seperated with a period/dot character (e.g.
         *  "parent.child.grandchild").
         *
         * @return
         */
        const TargetRegisterGroupDescriptor& getSubgroupDescriptor(std::string_view keyStr) const;
    };
}
