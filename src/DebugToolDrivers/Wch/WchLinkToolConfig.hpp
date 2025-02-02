#pragma once

#include <yaml-cpp/yaml.h>

#include "src/ProjectConfig.hpp"
#include "src/DebugToolDrivers/Protocols/RiscVDebug/DebugTranslatorConfig.hpp"

namespace DebugToolDrivers::Wch
{
    /**
     * Extending the generic DebugToolConfig struct to accommodate WCH-Link configuration parameters.
     */
    struct WchLinkToolConfig: public DebugToolConfig
    {
        bool exitIapMode = true;
        ::DebugToolDrivers::Protocols::RiscVDebug::DebugTranslatorConfig riscVDebugTranslatorConfig = {};

        explicit WchLinkToolConfig(const DebugToolConfig& toolConfig);
    };
}
