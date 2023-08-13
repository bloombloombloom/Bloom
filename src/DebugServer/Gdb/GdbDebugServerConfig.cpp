#include "GdbDebugServerConfig.hpp"

#include "src/Helpers/YamlUtilities.hpp"
#include "src/Logger/Logger.hpp"

namespace DebugServer::Gdb
{
    GdbDebugServerConfig::GdbDebugServerConfig(const DebugServerConfig& debugServerConfig)
        : DebugServerConfig(debugServerConfig)
    {
        if (debugServerConfig.debugServerNode["ipAddress"]) {
            if (!YamlUtilities::isCastable<std::string>(debugServerConfig.debugServerNode["ipAddress"])) {
                Logger::error(
                    "Invalid GDB debug server config parameter ('ipAddress') provided - must be a string. The "
                    "parameter will be ignored."
                );
            }

            this->listeningAddress = debugServerConfig.debugServerNode["ipAddress"].as<std::string>();
        }

        if (debugServerConfig.debugServerNode["port"]) {
            if (YamlUtilities::isCastable<std::uint16_t>(debugServerConfig.debugServerNode["port"])) {
                this->listeningPortNumber = debugServerConfig.debugServerNode["port"].as<std::uint16_t>();

            } else {
                Logger::error(
                    "Invalid GDB debug server config parameter ('port') provided - value must be castable to a 16-bit "
                    "unsigned integer. The parameter will be ignored."
                );
            }
        }
    }
}
