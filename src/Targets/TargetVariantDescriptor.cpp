#include "TargetVariantDescriptor.hpp"

#include <utility>

namespace Targets
{
    TargetVariantDescriptor::TargetVariantDescriptor(
        const std::string& name,
        const std::string& pinoutKey
    )
        : name(name)
        , pinoutKey(pinoutKey)
    {}
}