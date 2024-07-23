#pragma once

#include "src/ProjectConfig.hpp"

namespace Targets::RiscV
{
    /**
     * Extending the generic TargetConfig struct to accommodate RISC-V target configuration parameters.
     */
    struct RiscVTargetConfig: public TargetConfig
    {
    public:
        explicit RiscVTargetConfig(const TargetConfig& targetConfig);
    };
}
