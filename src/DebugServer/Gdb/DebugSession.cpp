#include "DebugSession.hpp"

#include "src/Logger/Logger.hpp"

namespace Bloom::DebugServer::Gdb
{
    DebugSession::DebugSession(
        Connection&& connection,
        const std::set<std::pair<Feature, std::optional<std::string>>>& supportedFeatures,
        const TargetDescriptor& targetDescriptor
    )
        : connection(std::move(connection))
        , supportedFeatures(supportedFeatures)
        , gdbTargetDescriptor(targetDescriptor)
    {
        this->supportedFeatures.insert({
            Feature::PACKET_SIZE, std::to_string(this->connection.getMaxPacketSize())
        });
    }

    void DebugSession::terminate() {

    }
}
