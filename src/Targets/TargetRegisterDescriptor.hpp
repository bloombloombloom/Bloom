#pragma once

#include <cstdint>
#include <atomic>
#include <string>
#include <optional>
#include <utility>
#include <map>
#include <set>

#include "TargetMemory.hpp"
#include "TargetAddressSpaceDescriptor.hpp"

namespace Targets
{
    using TargetRegisterDescriptorId = std::uint32_t;
    using TargetRegisterDescriptorIds = std::set<Targets::TargetRegisterDescriptorId>;

    struct TargetRegisterAccess
    {
        bool readable = false;
        bool writable = false;

        TargetRegisterAccess(
            bool readable,
            bool writable
        )
            : readable(readable)
            , writable(writable)
        {}

        bool operator == (const TargetRegisterAccess& other) const {
            return this->readable == other.readable
                && this->writable == other.writable
            ;
        }
    };

    struct TargetRegisterDescriptor
    {
    public:
        const TargetRegisterDescriptorId id;
        std::string key;
        std::string name;
        std::string addressSpaceKey;
        const TargetAddressSpaceDescriptorId addressSpaceDescriptorId;
        TargetMemoryAddress startAddress;
        TargetMemorySize size;
        TargetRegisterAccess access;
        std::optional<std::string> description;

        TargetRegisterDescriptor(
            const std::string& key,
            const std::string& name,
            const std::string& addressSpaceKey,
            TargetAddressSpaceDescriptorId addressSpaceDescriptorId,
            TargetMemoryAddress startAddress,
            TargetMemorySize size,
            TargetRegisterAccess access,
            std::optional<std::string> description
        )
            : id(++(TargetRegisterDescriptor::lastRegisterDescriptorId))
            , key(key)
            , name(name)
            , addressSpaceKey(addressSpaceKey)
            , addressSpaceDescriptorId(addressSpaceDescriptorId)
            , startAddress(startAddress)
            , size(size)
            , access(access)
            , description(description)
        {};

        bool operator == (const TargetRegisterDescriptor& other) const {
            return this->key == other.key
                && this->name == other.name
                && this->addressSpaceDescriptorId == other.addressSpaceDescriptorId
                && this->startAddress == other.startAddress
                && this->size == other.size
                && this->access == other.access
                && this->description == other.description
            ;
        }

        bool operator < (const TargetRegisterDescriptor& other) const {
            if (this->addressSpaceDescriptorId != other.addressSpaceDescriptorId) {
                /*
                 * If the registers are within different address spaces, there is no meaningful way to sort them, so
                 * we just sort by name.
                 */
                return this->name < other.name;
            }

            return this->startAddress < other.startAddress;
        }

    private:
        static inline std::atomic<TargetRegisterDescriptorId> lastRegisterDescriptorId = 0;
    };

    using TargetRegisterDescriptors = std::set<TargetRegisterDescriptor>;
    using TargetRegisterDescriptorMapping = std::map<TargetRegisterDescriptorId, TargetRegisterDescriptor>;
}
