#pragma once

#include <utility>
#include <vector>
#include <map>

namespace Bloom::Targets
{
    enum class TargetRegisterType: int
    {
        GENERAL_PURPOSE_REGISTER,
        PROGRAM_COUNTER,
        STACK_POINTER,
        STATUS_REGISTER,
        OTHER,
    };

    struct TargetRegisterDescriptor
    {
        std::optional<size_t> id;
        TargetRegisterType type = TargetRegisterType::OTHER;

        TargetRegisterDescriptor() = default;
        explicit TargetRegisterDescriptor(TargetRegisterType type): type(type) {};
        TargetRegisterDescriptor(size_t id, TargetRegisterType type): id(id), type(type) {};

        bool operator==(const TargetRegisterDescriptor& other) const {
            return this->id == other.id && this->type == other.type;
        }
    };

    struct TargetRegister
    {
        TargetRegisterDescriptor descriptor;
        std::vector<unsigned char> value;

        TargetRegister(TargetRegisterDescriptor descriptor, std::vector<unsigned char> value): value(std::move(value)),
        descriptor(descriptor) {};

        [[nodiscard]] size_t size() const {
            return this->value.size();
        }
    };

    using TargetRegisterMap = std::map<size_t, TargetRegister>;
    using TargetRegisters = std::vector<TargetRegister>;
    using TargetRegisterDescriptors = std::vector<TargetRegisterDescriptor>;
}

namespace std {
    /**
     * Hashing function for TargetRegisterDescriptor type.
     *
     * This is required in order to use TargetRegisterDescriptor as a key in an std::unordered_map (see the BiMap
     * class)
     */
    template<>
    class hash<Bloom::Targets::TargetRegisterDescriptor> {
    public:
        size_t operator()(const Bloom::Targets::TargetRegisterDescriptor& descriptor) const {
            return descriptor.id.value_or(0) + static_cast<size_t>(descriptor.type);
        }
    };
}
