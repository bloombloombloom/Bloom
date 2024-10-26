#include "EdbgToolConfig.hpp"

#include "src/Helpers/YamlUtilities.hpp"
#include "src/Logger/Logger.hpp"

namespace DebugToolDrivers::Microchip
{
    EdbgToolConfig::EdbgToolConfig(const DebugToolConfig& toolConfig)
        : DebugToolConfig(toolConfig)
    {
        const auto& toolNode = toolConfig.toolNode;

        const auto edbgDriverNode = toolNode["edbgDriver"];
        if (edbgDriverNode) {
            if (edbgDriverNode["cmsisCommandDelay"]) {
                if (YamlUtilities::isCastable<std::uint16_t>(edbgDriverNode["cmsisCommandDelay"])) {
                    this->cmsisCommandDelay = std::chrono::milliseconds{
                        edbgDriverNode["cmsisCommandDelay"].as<std::uint16_t>()
                    };

                } else {
                    Logger::error(
                        "Invalid EDBG driver config parameter ('cmsisCommandDelay') provided - must be a 16-bit "
                            "unsigned integer. The parameter will be ignored."
                    );
                }
            }
        }
    }
}
