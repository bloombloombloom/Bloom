#pragma once

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
    };

    struct TargetRegisterDescriptor
    {
        using IdType = size_t;
        std::optional<IdType> id;
        TargetRegisterType type = TargetRegisterType::GENERAL_PURPOSE_REGISTER;

        TargetRegisterDescriptor() {};
        TargetRegisterDescriptor(TargetRegisterType type): type(type) {};
        TargetRegisterDescriptor(IdType id, TargetRegisterType type): id(id), type(type) {};

        bool operator==(const TargetRegisterDescriptor& other) const {
            return this->id == other.id && this->type == other.type;
        }
    };

    struct TargetRegister
    {
        using IdType = size_t;
        using ValueType = std::vector<unsigned char>;
        std::optional<IdType> id;
        ValueType value;
        TargetRegisterDescriptor descriptor;

        TargetRegister(const ValueType& value): value(value) {};

        TargetRegister(TargetRegisterDescriptor descriptor, const ValueType& value): value(value),
        descriptor(descriptor) {};

        TargetRegister(IdType id, const ValueType& value): id(id), value(value) {
            this->descriptor.id = id;
        };

        IdType size() const {
            return this->value.size();
        }
    };

    using TargetRegisterMap = std::map<TargetRegister::IdType, TargetRegister>;
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
