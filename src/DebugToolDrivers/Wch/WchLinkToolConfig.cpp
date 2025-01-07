#include "WchLinkToolConfig.hpp"

namespace DebugToolDrivers::Wch
{
    WchLinkToolConfig::WchLinkToolConfig(const DebugToolConfig& toolConfig)
        : DebugToolConfig(toolConfig)
    {
        const auto& toolNode = toolConfig.toolNode;

        if (toolNode["exit_iap_mode"]) {
            this->exitIapMode = toolNode["exit_iap_mode"].as<bool>(this->exitIapMode);
        }

        if (toolNode["riscv_debug_translator"]) {
            this->riscVDebugTranslatorConfig = ::DebugToolDrivers::Protocols::RiscVDebugSpec::DebugTranslatorConfig{
                toolNode["riscv_debug_translator"]
            };
        }
    }
}
