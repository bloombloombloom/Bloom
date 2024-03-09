#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "TargetRegisterDescriptor.hpp"
#include "TargetMemory.hpp"

namespace Targets
{
    struct TargetRegister
    {
        TargetRegisterDescriptorId descriptorId;

        /**
         * Register values should be in MSB form
         */
        TargetMemoryBuffer value;

        TargetRegister(TargetRegisterDescriptorId descriptorId, TargetMemoryBuffer value)
            : descriptorId(descriptorId)
            , value(std::move(value))
        {};

        [[nodiscard]] std::size_t size() const {
            return this->value.size();
        }
    };

    using TargetRegisters = std::vector<TargetRegister>;
}
