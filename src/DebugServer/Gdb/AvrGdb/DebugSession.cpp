#include "DebugSession.hpp"

namespace DebugServer::Gdb::AvrGdb
{
    DebugSession::DebugSession(
        Connection&& connection,
        const std::set<std::pair<Feature, std::optional<std::string>>>& supportedFeatures,
        const GdbDebugServerConfig& serverConfig
    )
        : Gdb::DebugSession(std::move(connection), supportedFeatures, serverConfig)
    {}
}
