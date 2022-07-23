#include "GdbDebugServerConfig.hpp"

#include "src/Helpers/YamlUtilities.hpp"
#include "src/Logger/Logger.hpp"

namespace Bloom::DebugServer::Gdb
{
    GdbDebugServerConfig::GdbDebugServerConfig(const DebugServerConfig& debugServerConfig)
        : DebugServerConfig(debugServerConfig)
    {
        if (debugServerConfig.debugServerNode["ipAddress"]) {
            if (!YamlUtilities::isType<std::string>(debugServerConfig.debugServerNode["ipAddress"])) {
                Logger::error(
                    "Invalid GDB debug server config parameter ('ipAddress') provided - must be a string. The "
                    "parameter will be ignored."
                );
            }

            this->listeningAddress = debugServerConfig.debugServerNode["ipAddress"].as<std::string>();
        }

        if (debugServerConfig.debugServerNode["port"]) {
            this->listeningPortNumber = static_cast<std::uint16_t>(
                debugServerConfig.debugServerNode["port"].as<int>()
            );
        }
    }
}
