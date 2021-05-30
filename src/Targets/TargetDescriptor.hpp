#pragma once

#include <string>
#include <cstdint>
#include <vector>

#include "TargetVariant.hpp"

namespace Bloom::Targets
{
    struct TargetDescriptor
    {
        std::string name;
        std::string id;
        std::uint32_t ramSize;
        std::vector<TargetVariant> variants;
    };
}

Q_DECLARE_METATYPE(Bloom::Targets::TargetDescriptor)
