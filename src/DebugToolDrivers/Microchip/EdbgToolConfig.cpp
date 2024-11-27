#include "EdbgToolConfig.hpp"

#include "src/Helpers/YamlUtilities.hpp"
#include "src/Logger/Logger.hpp"

namespace DebugToolDrivers::Microchip
{
    EdbgToolConfig::EdbgToolConfig(const DebugToolConfig& toolConfig)
        : DebugToolConfig(toolConfig)
    {
        const auto& toolNode = toolConfig.toolNode;

        const auto edbgDriverNode = toolNode["edbg_driver"];
        if (edbgDriverNode) {
            if (edbgDriverNode["cmsis_command_delay"]) {
                if (YamlUtilities::isCastable<std::uint16_t>(edbgDriverNode["cmsis_command_delay"])) {
                    this->cmsisCommandDelay = std::chrono::milliseconds{
                        edbgDriverNode["cmsis_command_delay"].as<std::uint16_t>()
                    };

                } else {
                    Logger::error(
                        "Invalid EDBG driver config parameter ('cmsis_command_delay') provided - must be a 16-bit "
                            "unsigned integer. The parameter will be ignored."
                    );
                }
            }
        }
    }
}
