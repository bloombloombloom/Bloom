#pragma once

#include <optional>

#include "src/DebugToolDrivers/Wch/WchFirmwareVersion.hpp"
#include "src/DebugToolDrivers/Wch/WchGeneric.hpp"

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
