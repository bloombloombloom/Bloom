#pragma once

#include <cstdint>
#include <string>
#include <optional>
#include <map>
#include <set>
#include <vector>

#include "TargetMemory.hpp"
#include "TargetAddressSpaceDescriptor.hpp"
#include "TargetBitFieldDescriptor.hpp"

#include "src/Helpers/Pair.hpp"

namespace Targets
{
    using TargetRegisterDescriptorId = std::uint32_t;
    using TargetRegisterDescriptorIds = std::set<Targets::TargetRegisterDescriptorId>;

    enum class TargetRegisterType: std::uint8_t
    {
        GENERAL_PURPOSE_REGISTER,
        OTHER,
    };

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
        std::string key;
        std::string name;
        TargetAddressSpaceId addressSpaceId;
        std::string addressSpaceKey;
        TargetMemoryAddress startAddress;
        TargetMemorySize size;
        TargetRegisterType type;
        TargetRegisterAccess access;
        std::optional<std::string> description;
        std::map<std::string, TargetBitFieldDescriptor> bitFieldDescriptorsByKey;

        TargetRegisterDescriptor(
            const std::string& key,
            const std::string& name,
            const std::string& addressSpaceKey,
            TargetMemoryAddress startAddress,
            TargetMemorySize size,
            TargetRegisterType type,
            TargetRegisterAccess access,
            std::optional<std::string> description,
            std::map<std::string, TargetBitFieldDescriptor>&& bitFieldDescriptorsByKey
        );

        TargetRegisterDescriptor(const TargetRegisterDescriptor& other) = delete;
        TargetRegisterDescriptor& operator = (const TargetRegisterDescriptor& other) = delete;

        TargetRegisterDescriptor(TargetRegisterDescriptor&& other) noexcept = default;
        TargetRegisterDescriptor& operator = (TargetRegisterDescriptor&& other) = default;

        bool operator == (const TargetRegisterDescriptor& other) const;
        bool operator < (const TargetRegisterDescriptor& other) const;

        [[nodiscard]] std::optional<std::reference_wrapper<const TargetBitFieldDescriptor>> tryGetBitFieldDescriptor(
            const std::string& key
        ) const;
        [[nodiscard]] const TargetBitFieldDescriptor& getBitFieldDescriptor(const std::string& key) const;

        [[nodiscard]] TargetRegisterDescriptor clone() const;
    };

    using TargetRegisterDescriptors = std::vector<const TargetRegisterDescriptor*>;
    using TargetRegisterDescriptorAndValuePair = Pair<const TargetRegisterDescriptor&, TargetMemoryBuffer>;
    using TargetRegisterDescriptorAndValuePairs = std::vector<TargetRegisterDescriptorAndValuePair>;
}
