#include "TargetRegisterDescriptor.hpp"

#include "src/Services/StringService.hpp"
#include "TargetAddressSpaceDescriptor.hpp"

#include "src/Exceptions/InternalFatalErrorException.hpp"

namespace Targets
{
    TargetRegisterDescriptor::TargetRegisterDescriptor(
        const std::string& key,
        const std::string& name,
        const std::string& absoluteGroupKey,
        const std::string& peripheralKey,
        const std::string& addressSpaceKey,
        TargetMemoryAddress startAddress,
        TargetMemorySize size,
        TargetRegisterType type,
        TargetRegisterAccess access,
        std::optional<std::string> description,
        std::map<std::string, TargetBitFieldDescriptor>&& bitFieldDescriptorsByKey
    )
        : id(Services::StringService::generateUniqueInteger(peripheralKey + absoluteGroupKey + key))
        , key(key)
        , name(name)
        , absoluteGroupKey(absoluteGroupKey)
        , peripheralKey(peripheralKey)
        , addressSpaceId(TargetAddressSpaceDescriptor::generateId(addressSpaceKey))
        , addressSpaceKey(addressSpaceKey)
        , startAddress(startAddress)
        , size(size)
        , type(type)
        , access(access)
        , description(description)
        , bitFieldDescriptorsByKey(std::move(bitFieldDescriptorsByKey))
    {}

    bool TargetRegisterDescriptor::operator == (const TargetRegisterDescriptor& other) const {
        return this->id == other.id;
    }

    bool TargetRegisterDescriptor::operator < (const TargetRegisterDescriptor& other) const {
        if (this->addressSpaceKey != other.addressSpaceKey) {
            /*
             * If the registers are within different address spaces, there is no meaningful way to sort them, so
             * we just sort by name.
             */
            return this->name < other.name;
        }

        return this->startAddress < other.startAddress;
    }

    [[nodiscard]] std::optional<
        std::reference_wrapper<const TargetBitFieldDescriptor>
    > TargetRegisterDescriptor::tryGetBitFieldDescriptor(const std::string& key) const {
        const auto bitFieldIt = this->bitFieldDescriptorsByKey.find(key);

        if (bitFieldIt == this->bitFieldDescriptorsByKey.end()) {
            return std::nullopt;
        }

        return std::cref(bitFieldIt->second);
    }

    [[nodiscard]] const TargetBitFieldDescriptor& TargetRegisterDescriptor::getBitFieldDescriptor(
        const std::string& key
    ) const {
        const auto bitField = this->tryGetBitFieldDescriptor(key);
        if (!bitField.has_value()) {
            throw Exceptions::InternalFatalErrorException{
                "Failed to get bit field descriptor \"" + std::string{key} + "\" from register - bit field not found"
            };
        }

        return bitField->get();
    }

    TargetRegisterDescriptor TargetRegisterDescriptor::clone() const {
        auto output = TargetRegisterDescriptor{
            this->key,
            this->name,
            this->absoluteGroupKey,
            this->peripheralKey,
            this->addressSpaceKey,
            this->startAddress,
            this->size,
            this->type,
            this->access,
            this->description,
            {}
        };

        for (const auto& [key, descriptor] : this->bitFieldDescriptorsByKey) {
            output.bitFieldDescriptorsByKey.emplace(key, descriptor.clone());
        }

        return output;
    }
}
