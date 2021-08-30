#pragma once

#include <string>
#include <optional>
#include <cstdint>
#include <utility>
#include <vector>
#include <map>

#include "TargetMemory.hpp"

namespace Bloom::Targets
{
    enum class TargetRegisterType: std::uint8_t
    {
        GENERAL_PURPOSE_REGISTER,
        PROGRAM_COUNTER,
        STACK_POINTER,
        STATUS_REGISTER,
        OTHER,
    };

    struct TargetRegisterDescriptor
    {
        friend std::hash<Bloom::Targets::TargetRegisterDescriptor>;

    private:
        mutable std::optional<std::size_t> cachedHash;

        std::size_t getHash() const;

    public:
        std::optional<std::uint32_t> startAddress;
        std::uint32_t size = 0;
        TargetRegisterType type = TargetRegisterType::OTHER;

        std::optional<std::string> name = "";
        std::optional<std::string> groupName;
        std::optional<std::string> description;

        bool readable = false;
        bool writable = false;

        TargetRegisterDescriptor() = default;
        explicit TargetRegisterDescriptor(TargetRegisterType type): type(type) {};

        bool operator == (const TargetRegisterDescriptor& other) const {
            return this->getHash() == other.getHash();
        }

        bool operator < (const TargetRegisterDescriptor& other) const {
            if (this->type == other.type) {
                return this->startAddress.value_or(0) < other.startAddress.value_or(0);

            } else {
                /*
                 * If the registers are of different type, there is no meaningful way to sort them, so we just use
                 * the unique hash.
                 */
                return this->getHash() < other.getHash();
            }
        }
    };

    struct TargetRegister
    {
        TargetRegisterDescriptor descriptor;
        TargetMemoryBuffer value;

        TargetRegister(TargetRegisterDescriptor descriptor, std::vector<unsigned char> value): value(std::move(value)),
        descriptor(std::move(descriptor)) {};

        [[nodiscard]] std::size_t size() const {
            return this->value.size();
        }
    };

    using TargetRegisters = std::vector<TargetRegister>;
    using TargetRegisterDescriptors = std::vector<TargetRegisterDescriptor>;
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
    class hash<Bloom::Targets::TargetRegisterDescriptor>
    {
    public:
        std::size_t operator()(const Bloom::Targets::TargetRegisterDescriptor& descriptor) const {
            return descriptor.getHash();
        }
    };
}
