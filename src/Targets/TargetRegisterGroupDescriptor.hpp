#pragma once

#include <cstdint>
#include <string>
#include <map>
#include <functional>
#include <optional>
#include <string_view>
#include <initializer_list>
#include <ranges>
#include <concepts>

#include "TargetRegisterDescriptor.hpp"
#include "src/Helpers/Pair.hpp"

namespace Targets
{
    struct TargetRegisterGroupDescriptor
    {
    public:
        std::string key;
        std::string name;
        std::string addressSpaceKey;
        std::optional<std::string> description;
        std::map<std::string, TargetRegisterDescriptor, std::less<void>> registerDescriptorsByKey;
        std::map<std::string, TargetRegisterGroupDescriptor, std::less<void>> subgroupDescriptorsByKey;

        TargetRegisterGroupDescriptor(
            const std::string& key,
            const std::string& name,
            const std::string& addressSpaceKey,
            const std::optional<std::string>& description,
            std::map<std::string, TargetRegisterDescriptor, std::less<void>>&& registerDescriptorsByKey,
            std::map<std::string, TargetRegisterGroupDescriptor, std::less<void>>&& subgroupDescriptorsByKey
        );

        TargetRegisterGroupDescriptor(const TargetRegisterGroupDescriptor& other) = delete;
        TargetRegisterGroupDescriptor& operator = (const TargetRegisterGroupDescriptor& other) = delete;

        TargetRegisterGroupDescriptor(TargetRegisterGroupDescriptor&& other) noexcept = default;
        TargetRegisterGroupDescriptor& operator = (TargetRegisterGroupDescriptor&& other) = default;

        /**
         * Calculates the start address of the register group.
         *
         * Excessive calls to this function is discouraged, as the implementation is quite slow.
         *
         * @return
         */
        [[nodiscard]] TargetMemoryAddress startAddress() const;

        /**
         * Calculates the size of this register group.
         *
         * Excessive calls to this function is discouraged, as the implementation is quite slow.
         *
         * @return
         */
        [[nodiscard]] TargetMemorySize size() const;

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
         *  "grandchild.parent.child").
         *
         * @return
         */
        [[nodiscard]] std::optional<
            std::reference_wrapper<const TargetRegisterGroupDescriptor>
        > tryGetSubgroupDescriptor(std::string_view keyStr) const;

        /**
         * Fetches a subgroup with a set of keys in the form of a string in "dot notation". If the subgroup is not
         * found, an InternalFatalErrorException is thrown.
         *
         * @param keyStr
         *  A string in "dot notation", containing each key seperated with a period/dot character (e.g.
         *  "grandchild.parent.child").
         *
         * @return
         */
        [[nodiscard]] const TargetRegisterGroupDescriptor& getSubgroupDescriptor(std::string_view keyStr) const;

        /**
         * Register descriptor lookup by key.
         *
         * @param key
         *
         * @return
         *  The register descriptor matching the given key, if it exists. Otherwise, std::nullopt.
         */
        [[nodiscard]] std::optional<std::reference_wrapper<const TargetRegisterDescriptor>> tryGetRegisterDescriptor(
            const std::string& key
        ) const;

        /**
         * Performs a lookup of each key in the given set of keys, and returns the first matching register descriptor.
         *
         * @param keys
         *
         * @return
         *  std::nullopt if all keys do not map to any register descriptors.
         */
        [[nodiscard]] std::optional<
            std::reference_wrapper<const TargetRegisterDescriptor>
        > tryGetFirstRegisterDescriptor(std::initializer_list<std::string>&& keys) const;

        [[nodiscard]] const TargetRegisterDescriptor& getRegisterDescriptor(const std::string& key) const;

        /**
         * Finds the first register descriptor containing the bit field with the given key.
         *
         * This function will throw an InternalFatalErrorException if the bit field was not found in any of the
         * registers within this register group.
         *
         * @param bitFieldKey
         *
         * @return
         *  A Pair object with Pair::first being the register descriptor in which the bit field was found, and
         *  Pair::second being the bit field descriptor.
         */
        [[nodiscard]] Pair<
            const TargetRegisterDescriptor&,
            const TargetBitFieldDescriptor&
        > getRegisterBitFieldDescriptorPair(const std::string& bitFieldKey) const;

        [[nodiscard]] TargetRegisterGroupDescriptor clone() const;
    };
}
