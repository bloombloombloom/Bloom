#include "TargetPinDescriptor.hpp"

#include "src/Services/StringService.hpp"

namespace Targets
{
    TargetPinDescriptor::TargetPinDescriptor(
        const std::string& position,
        const std::optional<std::string>& padKey
    )
        : position(position)
        , numericPosition(
            Services::StringService::isNumeric(this->position)
                ? Services::StringService::toUint16(this->position, 10)
                : 0
        )
        , padKey(padKey)
    {}
}
