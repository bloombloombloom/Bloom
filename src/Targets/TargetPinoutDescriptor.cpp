#include "TargetPinoutDescriptor.hpp"

#include <utility>

namespace Targets
{
    TargetPinoutDescriptor::TargetPinoutDescriptor(
        const std::string& key,
        const std::string& name,
        TargetPinoutType type,
        std::vector<TargetPinDescriptor>&& pinDescriptors
    )
        : key(key)
        , name(name)
        , type(type)
        , pinDescriptors(std::move(pinDescriptors))
    {}

    bool TargetPinoutDescriptor::operator == (const TargetPinoutDescriptor& other) const {
        return this->key == other.key;
    }
}
