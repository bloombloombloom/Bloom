#include "TargetPinDescriptor.hpp"

namespace Targets
{
    TargetPinDescriptor::TargetPinDescriptor(
        const std::string& padName,
        const std::string& position,
        TargetPinType type
    )
        : padName(padName)
        , position(position)
        , type(type)
    {}
}
