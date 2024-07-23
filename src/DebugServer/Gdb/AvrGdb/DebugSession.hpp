#pragma once

#include "src/DebugServer/Gdb/DebugSession.hpp"

#include "TargetDescriptor.hpp"

namespace DebugServer::Gdb::AvrGdb
{
    class DebugSession final: public Gdb::DebugSession
    {
    public:
        DebugSession(
            Connection&& connection,
            const std::set<std::pair<Feature, std::optional<std::string>>>& supportedFeatures,
            const GdbDebugServerConfig& serverConfig
        );
    };
}
