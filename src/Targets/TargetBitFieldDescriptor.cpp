#include "TargetBitFieldDescriptor.hpp"

namespace Targets
{
    TargetBitFieldDescriptor::TargetBitFieldDescriptor(
        const std::string& key,
        const std::string& name,
        std::uint64_t mask,
        std::optional<std::string> description
    )
        : key(key)
        , name(name)
        , mask(mask)
        , description(description)
    {}

    TargetBitFieldDescriptor TargetBitFieldDescriptor::clone() const {
        return {*this};
    }
}
