#include "GdbDebugServerConfig.hpp"

namespace Bloom::DebugServer::Gdb
{
    GdbDebugServerConfig::GdbDebugServerConfig(const DebugServerConfig& debugServerConfig)
        : DebugServerConfig(debugServerConfig)
    {
        if (debugServerConfig.jsonObject.contains("ipAddress")) {
            this->listeningAddress = debugServerConfig.jsonObject.value("ipAddress").toString().toStdString();
        }

        if (debugServerConfig.jsonObject.contains("port")) {
            const auto portValue = debugServerConfig.jsonObject.value("port");
            this->listeningPortNumber = static_cast<std::uint16_t>(
                portValue.isString() ? portValue.toString().toInt(nullptr, 10) : portValue.toInt()
            );
        }
    }
}
