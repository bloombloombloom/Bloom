#pragma once

#include <optional>

#include "src/Targets/RiscV/RiscVTargetConfig.hpp"

namespace Targets::RiscV
{
    struct WchRiscVTargetConfig: public RiscVTargetConfig
    {
    public:
        explicit WchRiscVTargetConfig(const RiscVTargetConfig& targetConfig);

        std::optional<std::string> programSegmentKey;
    };
}
