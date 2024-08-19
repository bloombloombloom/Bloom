#include "TargetVariantDescriptor.hpp"

#include <utility>

namespace Targets
{
    TargetVariantDescriptor::TargetVariantDescriptor(
        const std::string& key,
        const std::string& name,
        const std::string& pinoutKey
    )
        : key(key)
        , name(name)
        , pinoutKey(pinoutKey)
    {}
}
