#pragma once

#include <optional>
#include <chrono>
#include <yaml-cpp/yaml.h>

#include "src/ProjectConfig.hpp"

namespace DebugToolDrivers::Microchip
{
    /**
     * Extending the generic DebugToolConfig struct to accommodate EDBG configuration parameters.
     */
    struct EdbgToolConfig: public DebugToolConfig
    {
    public:
        std::optional<std::chrono::milliseconds> cmsisCommandDelay = std::nullopt;

        explicit EdbgToolConfig(const DebugToolConfig& toolConfig);
    };
}
