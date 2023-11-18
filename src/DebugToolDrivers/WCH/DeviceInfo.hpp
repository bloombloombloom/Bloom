#pragma once

#include <optional>

#include "src/DebugToolDrivers/WCH/WchFirmwareVersion.hpp"
#include "src/DebugToolDrivers/WCH/WchGeneric.hpp"

namespace DebugToolDrivers::Wch
{
    class DeviceInfo
    {
    public:
        WchFirmwareVersion firmwareVersion;
        std::optional<WchLinkVariant> variant;

        explicit DeviceInfo(WchFirmwareVersion firmwareVersion, std::optional<WchLinkVariant> variant)
            : firmwareVersion(firmwareVersion)
            , variant(variant)
        {}
    };
}
