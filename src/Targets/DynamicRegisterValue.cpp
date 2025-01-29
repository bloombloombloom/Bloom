#include "DynamicRegisterValue.hpp"

#include <bitset>
#include <cassert>

namespace Targets
{
    DynamicRegisterValue::DynamicRegisterValue(const TargetRegisterDescriptorAndValuePair& pair)
        : DynamicRegisterValue(pair.first, pair.second)
    {}

    DynamicRegisterValue::DynamicRegisterValue(
        const TargetRegisterDescriptor& registerDescriptor,
        TargetMemoryBufferSpan value
    )
        : registerDescriptor(registerDescriptor)
        , value(std::uint64_t{0})
    {
        assert(registerDescriptor.size <= 8);
        assert(value.size() == this->registerDescriptor.size);

        for (const auto byte : value) {
            this->value = (this->value << 8) | byte;
        }
    }

    DynamicRegisterValue::DynamicRegisterValue(const TargetRegisterDescriptor& registerDescriptor, std::uint64_t value)
        : registerDescriptor(registerDescriptor)
        , value(value)
    {
        assert(registerDescriptor.size <= 8);
    }

    DynamicRegisterValue& DynamicRegisterValue::operator = (const DynamicRegisterValue& other) {
        assert(other.registerDescriptor == this->registerDescriptor);
        this->value = other.value;

        return *this;
    }

    void DynamicRegisterValue::setBitField(
        const TargetBitFieldDescriptor& bitFieldDescriptor,
        std::uint64_t inputValue
    ) {
        auto valueBitset = std::bitset<64>{this->value};

        const auto maskBitset = std::bitset<64>{bitFieldDescriptor.mask};
        auto inputValueIndex = std::size_t{0};
        for (auto maskIndex = std::size_t{0}; maskIndex < maskBitset.size(); ++maskIndex) {
            if (maskBitset[maskIndex]) {
                valueBitset.set(maskIndex, static_cast<bool>(inputValue & (0x01 << inputValueIndex++)));
            }
        }

        this->value = valueBitset.to_ullong();
    }

    void DynamicRegisterValue::setBitField(const std::string& bitFieldKey, std::uint64_t value) {
        return this->setBitField(this->registerDescriptor.getBitFieldDescriptor(bitFieldKey), value);
    }

    std::uint64_t DynamicRegisterValue::bitField(const TargetBitFieldDescriptor& bitFieldDescriptor) const {
        auto output = std::bitset<64>{0};

        const auto maskBitset = std::bitset<64>{bitFieldDescriptor.mask};
        auto outputIndex = std::size_t{0};
        for (auto maskIndex = std::size_t{0}; maskIndex < maskBitset.size(); ++maskIndex) {
            if (maskBitset[maskIndex]) {
                output.set(outputIndex++, static_cast<bool>(this->value & (0x01 << maskIndex)));
            }
        }

        return output.to_ullong();
    }

    std::uint64_t DynamicRegisterValue::bitField(const std::string& bitFieldKey) const {
        return this->bitField(this->registerDescriptor.getBitFieldDescriptor(bitFieldKey));
    }

    TargetMemoryBuffer DynamicRegisterValue::data() const {
        auto output = TargetMemoryBuffer(this->registerDescriptor.size, 0x00);

        for (auto i = TargetMemorySize{0}; i < this->registerDescriptor.size; ++i) {
            output[i] = static_cast<unsigned char>(this->value >> ((this->registerDescriptor.size - i - 1) * 8));
        }

        return output;
    }
}
