#pragma once

#include <cstdint>
#include <string>

#include "TargetRegisterDescriptor.hpp"
#include "TargetBitFieldDescriptor.hpp"
#include "TargetMemory.hpp"

namespace Targets
{
    class DynamicRegisterValue
    {
    public:
        const TargetRegisterDescriptor& registerDescriptor;
        std::uint64_t value;

        explicit DynamicRegisterValue(const TargetRegisterDescriptorAndValuePair& pair);
        DynamicRegisterValue(const TargetRegisterDescriptor& registerDescriptor, TargetMemoryBufferSpan value);
        DynamicRegisterValue(const TargetRegisterDescriptor& registerDescriptor, std::uint64_t value);

        void setBitField(const TargetBitFieldDescriptor& bitFieldDescriptor, std::uint64_t inputValue);
        void setBitField(const std::string& bitFieldKey, std::uint64_t inputValue);

        [[nodiscard]] std::uint64_t bitField(const TargetBitFieldDescriptor& bitFieldDescriptor) const;
        [[nodiscard]] std::uint64_t bitField(const std::string& bitFieldKey) const;
        [[nodiscard]] bool bitFieldFlag(const TargetBitFieldDescriptor& bitFieldDescriptor) const;
        [[nodiscard]] bool bitFieldFlag(const std::string& bitFieldKey) const;

        [[nodiscard]] TargetMemoryBuffer data() const;
    };
}
