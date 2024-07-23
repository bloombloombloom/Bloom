#include "TargetPeripheralSignalDescriptor.hpp"

namespace Targets
{
    TargetPeripheralSignalDescriptor::TargetPeripheralSignalDescriptor(
        const std::string& padName,
        const std::optional<std::uint16_t>& index
    )
        : padName(padName)
        , index(index)
    {}

    TargetPeripheralSignalDescriptor TargetPeripheralSignalDescriptor::clone() const {
        return {
            this->padName,
            this->index
        };
    }
}
