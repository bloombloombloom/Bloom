#include "TargetPeripheralSignalDescriptor.hpp"

namespace Targets
{
    TargetPeripheralSignalDescriptor::TargetPeripheralSignalDescriptor(
        const std::string& padKey,
        const std::optional<std::uint16_t>& index
    )
        : padId(TargetPadDescriptor::generateId(padKey))
        , padKey(padKey)
        , index(index)
    {}

    TargetPeripheralSignalDescriptor TargetPeripheralSignalDescriptor::clone() const {
        return {
            this->padKey,
            this->index
        };
    }
}
