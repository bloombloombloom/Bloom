#pragma once

#include <cstdint>
#include <atomic>
#include <string>
#include <optional>
#include <utility>
#include <vector>
#include <map>
#include <set>

#include "TargetMemory.hpp"

namespace Targets
{
    using TargetRegisterDescriptorId = std::uint32_t;
    using TargetRegisterDescriptorIds = std::set<Targets::TargetRegisterDescriptorId>;

    enum class TargetRegisterType: std::uint8_t
    {
        GENERAL_PURPOSE_REGISTER,
        PROGRAM_COUNTER,
        STACK_POINTER,
        STATUS_REGISTER,
        PORT_REGISTER,
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
    };

    struct TargetRegisterDescriptor
    {
    public:
        TargetRegisterDescriptorId id;
        std::optional<TargetMemoryAddress> startAddress;
        TargetMemorySize size;
        TargetRegisterType type;
        TargetMemoryType memoryType;

        std::optional<std::string> name;
        std::optional<std::string> groupName;
        std::optional<std::string> description;

        TargetRegisterAccess access;

        TargetRegisterDescriptor(
            TargetRegisterType type,
            std::optional<TargetMemoryAddress> startAddress,
            TargetMemorySize size,
            TargetMemoryType memoryType,
            std::optional<std::string> name,
            std::optional<std::string> groupName,
            std::optional<std::string> description,
            TargetRegisterAccess access
        )
            : id(++(TargetRegisterDescriptor::lastRegisterDescriptorId))
            , type(type)
            , startAddress(startAddress)
            , size(size)
            , memoryType(memoryType)
            , name(name)
            , groupName(groupName)
            , description(description)
            , access(access)
        {};

        bool operator == (const TargetRegisterDescriptor& other) const {
            return this->getHash() == other.getHash();
        }

        bool operator < (const TargetRegisterDescriptor& other) const {
            if (this->type == other.type) {
                return this->startAddress.value_or(0) < other.startAddress.value_or(0);
            }

            /*
             * If the registers are of different type, there is no meaningful way to sort them, so we just use
             * the unique hash.
             */
            return this->getHash() < other.getHash();
        }

    private:
        mutable std::optional<std::size_t> cachedHash;
        static inline std::atomic<TargetRegisterDescriptorId> lastRegisterDescriptorId = 0;
        std::size_t getHash() const;

        friend std::hash<Targets::TargetRegisterDescriptor>;
    };

    struct TargetRegister
    {
        TargetRegisterDescriptorId descriptorId;
        TargetMemoryBuffer value;

        TargetRegister(TargetRegisterDescriptorId descriptorId, std::vector<unsigned char> value)
            : value(std::move(value))
            , descriptorId(descriptorId)
        {};

        [[nodiscard]] std::size_t size() const {
            return this->value.size();
        }
    };

    using TargetRegisters = std::vector<TargetRegister>;
    using TargetRegisterDescriptors = std::set<TargetRegisterDescriptor>;
    using TargetRegisterDescriptorMapping = std::map<TargetRegisterDescriptorId, TargetRegisterDescriptor>;
}

namespace std
{
    /**
     * Hashing function for TargetRegisterDescriptor type.
     *
     * This is required in order to use TargetRegisterDescriptor as a key in an std::unordered_map (see the BiMap
     * class)
     */
    template<>
    class hash<Targets::TargetRegisterDescriptor>
    {
    public:
        std::size_t operator()(const Targets::TargetRegisterDescriptor& descriptor) const {
            return descriptor.getHash();
        }
    };
}
