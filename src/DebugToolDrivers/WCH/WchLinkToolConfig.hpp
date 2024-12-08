#pragma once

#include <yaml-cpp/yaml.h>

#include "src/ProjectConfig.hpp"
#include "src/DebugToolDrivers/Protocols/RiscVDebugSpec/DebugTranslatorConfig.hpp"

namespace DebugToolDrivers::Wch
{
    /**
     * Extending the generic DebugToolConfig struct to accommodate WCH-Link configuration parameters.
     */
    struct WchLinkToolConfig: public DebugToolConfig
    {
        bool exitIapMode = true;
        ::DebugToolDrivers::Protocols::RiscVDebugSpec::DebugTranslatorConfig riscVDebugTranslatorConfig = {};

        explicit WchLinkToolConfig(const DebugToolConfig& toolConfig);
    };
}
