#include "DebugSession.hpp"

#include "src/EventManager/EventManager.hpp"

namespace Bloom::DebugServer::Gdb
{
    DebugSession::DebugSession(
        Connection&& connection,
        const std::set<std::pair<Feature, std::optional<std::string>>>& supportedFeatures,
        const TargetDescriptor& targetDescriptor,
        const GdbDebugServerConfig& serverConfig
    )
        : connection(std::move(connection))
        , supportedFeatures(supportedFeatures)
        , gdbTargetDescriptor(targetDescriptor)
        , serverConfig(serverConfig)
    {
        this->supportedFeatures.insert({
            Feature::PACKET_SIZE, std::to_string(Connection::ABSOLUTE_MAXIMUM_PACKET_READ_SIZE)
        });

        EventManager::triggerEvent(std::make_shared<Events::DebugSessionStarted>());
    }

    DebugSession::~DebugSession() {
        EventManager::triggerEvent(std::make_shared<Events::DebugSessionFinished>());
    }
}
