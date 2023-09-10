#include "DebugSession.hpp"

namespace DebugServer::Gdb::AvrGdb
{
    DebugSession::DebugSession(
        Connection&& connection,
        const std::set<std::pair<Feature, std::optional<std::string>>>& supportedFeatures,
        const TargetDescriptor& targetDescriptor,
        const GdbDebugServerConfig& serverConfig
    )
        : Gdb::DebugSession(
            std::move(connection),
            supportedFeatures,
            targetDescriptor,
            serverConfig
        )
    {}
}
