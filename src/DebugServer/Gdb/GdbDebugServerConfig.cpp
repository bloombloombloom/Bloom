#include "GdbDebugServerConfig.hpp"

#include "src/Helpers/YamlUtilities.hpp"
#include "src/Logger/Logger.hpp"

namespace DebugServer::Gdb
{
    GdbDebugServerConfig::GdbDebugServerConfig(const DebugServerConfig& debugServerConfig)
        : DebugServerConfig(debugServerConfig)
    {
        if (debugServerConfig.debugServerNode["ip_address"]) {
            if (YamlUtilities::isCastable<std::string>(debugServerConfig.debugServerNode["ip_address"])) {
                this->listeningAddress = debugServerConfig.debugServerNode["ip_address"].as<std::string>();

            } else {
                Logger::error(
                    "Invalid GDB debug server config parameter ('ip_address') provided - must be a string. The "
                        "parameter will be ignored."
                );
            }
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

        if (debugServerConfig.debugServerNode["range_stepping"]) {
            if (YamlUtilities::isCastable<bool>(debugServerConfig.debugServerNode["range_stepping"])) {
                this->rangeStepping = debugServerConfig.debugServerNode["range_stepping"].as<bool>();

            } else {
                Logger::error(
                    "Invalid GDB debug server config parameter ('range_stepping') provided - value must be castable to "
                        "a boolean. The parameter will be ignored."
                );
            }
        }

        if (debugServerConfig.debugServerNode["packet_acknowledgement"]) {
            if (YamlUtilities::isCastable<bool>(debugServerConfig.debugServerNode["packet_acknowledgement"])) {
                this->packetAcknowledgement = debugServerConfig.debugServerNode["packet_acknowledgement"].as<bool>();

            } else {
                Logger::error(
                    "Invalid GDB debug server config parameter ('packet_acknowledgement') provided - value must be"
                        " castable to a boolean. The parameter will be ignored."
                );
            }
        }
    }
}
