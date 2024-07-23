#include "TargetRegisterGroupDescriptor.hpp"

#include <cassert>
#include <numeric>

#include "src/Services/StringService.hpp"
#include "src/Exceptions/InternalFatalErrorException.hpp"

namespace Targets
{
    TargetRegisterGroupDescriptor::TargetRegisterGroupDescriptor(
        const std::string& key,
        const std::string& name,
        const std::string& addressSpaceKey,
        const std::optional<std::string>& description,
        std::map<std::string, TargetRegisterDescriptor, std::less<void>>&& registerDescriptorsByKey,
        std::map<std::string, TargetRegisterGroupDescriptor, std::less<void>>&& subgroupDescriptorsByKey
    )
        : key(key)
        , name(name)
        , addressSpaceKey(addressSpaceKey)
        , description(description)
        , registerDescriptorsByKey(std::move(registerDescriptorsByKey))
        , subgroupDescriptorsByKey(std::move(subgroupDescriptorsByKey))
    {}

    TargetMemoryAddress TargetRegisterGroupDescriptor::startAddress() const {
        assert(!this->registerDescriptorsByKey.empty() || !this->subgroupDescriptorsByKey.empty());

        auto startAddress = TargetMemoryAddress{-1U};

        for (const auto& [key, registerDescriptor] : this->registerDescriptorsByKey) {
            if (registerDescriptor.startAddress < startAddress) {
                startAddress = registerDescriptor.startAddress;
            }
        }

        for (const auto& [key, groupDescriptor] : this->subgroupDescriptorsByKey) {
            const auto groupStartAddress = groupDescriptor.startAddress();
            if (groupStartAddress < startAddress) {
                startAddress = groupStartAddress;
            }
        }

        return startAddress;
    }

    TargetMemorySize TargetRegisterGroupDescriptor::size() const {
        return std::accumulate(
            this->registerDescriptorsByKey.begin(),
            this->registerDescriptorsByKey.end(),
            TargetMemorySize{0},
            [] (TargetMemorySize size, const decltype(this->registerDescriptorsByKey)::value_type& pair) {
                return size + pair.second.size;
            }
        ) + std::accumulate(
            this->subgroupDescriptorsByKey.begin(),
            this->subgroupDescriptorsByKey.end(),
            TargetMemorySize{0},
            [] (TargetMemorySize size, const decltype(this->subgroupDescriptorsByKey)::value_type& pair) {
                return size + pair.second.size();
            }
        );
    }

    std::optional<
        std::reference_wrapper<const TargetRegisterGroupDescriptor>
    > TargetRegisterGroupDescriptor::tryGetSubgroupDescriptor(std::string_view keyStr) const {
        return this->tryGetSubgroupDescriptor(Services::StringService::split(keyStr, '.'));
    }

    const TargetRegisterGroupDescriptor& TargetRegisterGroupDescriptor::getSubgroupDescriptor(
        std::string_view keyStr
    ) const {
        const auto subgroup = this->tryGetSubgroupDescriptor(keyStr);
        if (!subgroup.has_value()) {
            throw Exceptions::InternalFatalErrorException{
                "Failed to get subgroup \"" + std::string{keyStr} + "\" from register group - subgroup not found"
            };
        }

        return subgroup->get();
    }

    [[nodiscard]] std::optional<
        std::reference_wrapper<const TargetRegisterDescriptor>
    > TargetRegisterGroupDescriptor::tryGetRegisterDescriptor(const std::string& key) const {
        const auto registerIt = this->registerDescriptorsByKey.find(key);

        if (registerIt == this->registerDescriptorsByKey.end()) {
            return std::nullopt;
        }

        return std::cref(registerIt->second);
    }

    std::optional<
        std::reference_wrapper<const TargetRegisterDescriptor>
    > TargetRegisterGroupDescriptor::tryGetFirstRegisterDescriptor(std::initializer_list<std::string>&& keys) const {
        for (const auto& key : keys) {
            auto descriptor = this->tryGetRegisterDescriptor(key);
            if (descriptor.has_value()) {
                return descriptor;
            }
        }

        return std::nullopt;
    }

    [[nodiscard]] const TargetRegisterDescriptor& TargetRegisterGroupDescriptor::getRegisterDescriptor(
        const std::string& key
    ) const {
        const auto reg = this->tryGetRegisterDescriptor(key);
        if (!reg.has_value()) {
            throw Exceptions::InternalFatalErrorException{
                "Failed to get register descriptor \"" + std::string{key} + "\" from register group - register "
                    "descriptor not found"
            };
        }

        return reg->get();
    }

    Pair<
        const TargetRegisterDescriptor&,
        const TargetBitFieldDescriptor&
    > TargetRegisterGroupDescriptor::getRegisterBitFieldDescriptorPair(const std::string& bitFieldKey) const {
        for (const auto& [regKey, regDescriptor] : this->registerDescriptorsByKey) {
            const auto bitFieldDescriptor = regDescriptor.tryGetBitFieldDescriptor(bitFieldKey);
            if (bitFieldDescriptor.has_value()) {
                return {regDescriptor, bitFieldDescriptor->get()};
            }
        }

        throw Exceptions::InternalFatalErrorException{
            "Failed to find bit field \"" + std::string{bitFieldKey} + "\" within register group"
        };
    }

    TargetRegisterGroupDescriptor TargetRegisterGroupDescriptor::clone() const {
        auto output = TargetRegisterGroupDescriptor{
            this->key,
            this->name,
            this->addressSpaceKey,
            this->description,
            {},
            {}
        };

        for (const auto& [key, descriptor] : this->subgroupDescriptorsByKey) {
            output.subgroupDescriptorsByKey.emplace(key, descriptor.clone());
        }

        for (const auto& [key, descriptor] : this->registerDescriptorsByKey) {
            output.registerDescriptorsByKey.emplace(key, descriptor.clone());
        }

        return output;
    }
}
