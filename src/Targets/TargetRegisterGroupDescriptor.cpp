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
        TargetAddressSpaceDescriptorId addressSpaceDescriptorId,
        const std::optional<std::string>& description,
        const std::map<std::string, TargetRegisterDescriptor>& registerDescriptorsByKey,
        const std::map<std::string, TargetRegisterGroupDescriptor, std::less<void>>& subgroupDescriptorsByKey
    )
        : key(key)
        , name(name)
        , addressSpaceKey(addressSpaceKey)
        , addressSpaceDescriptorId(addressSpaceDescriptorId)
        , description(description)
        , registerDescriptorsByKey(registerDescriptorsByKey)
        , subgroupDescriptorsByKey(subgroupDescriptorsByKey)
    {}

    TargetMemoryAddress TargetRegisterGroupDescriptor::startAddress() const {
        assert(!this->registerDescriptorsByKey.empty() || !this->subgroupDescriptorsByKey.empty());

        auto startAddress = TargetMemoryAddress{0};

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
            throw Exceptions::InternalFatalErrorException(
                "Failed to get subgroup \"" + std::string(keyStr) + "\" from register group - subgroup not found"
            );
        }

        return subgroup->get();
    }
}
