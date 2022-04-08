#pragma once

#include <string>
#include <set>

#include "CommandPacket.hpp"
#include "../Feature.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    /**
     * The SupportedFeaturesQuery command packet is a query from the GDB client, requesting a list of GDB features
     * supported by the GDB server. The body of this packet also contains a list GDB features that are supported or
     * unsupported by the GDB client.
     *
     * The command packet is identified by its 'qSupported' prefix in the command packet data. Following the prefix is
     * a list of GDB features that are supported/unsupported by the client. For more info on this command
     * packet, see the GDP RSP documentation.
     *
     * Responses to this command packet should take the form of a ResponsePackets::SupportedFeaturesResponse.
     */
    class SupportedFeaturesQuery: public CommandPacket
    {
    public:
        explicit SupportedFeaturesQuery(const RawPacketType& rawPacket);

        [[nodiscard]] bool isFeatureSupported(const Feature& feature) const {
            return this->supportedFeatures.find(feature) != this->supportedFeatures.end();
        }

        [[nodiscard]] const std::set<Feature>& getSupportedFeatures() const {
            return this->supportedFeatures;
        }

        void handle(DebugSession& debugSession, TargetControllerConsole& targetControllerConsole) override;

    private:
        std::set<Feature> supportedFeatures;
    };
}
