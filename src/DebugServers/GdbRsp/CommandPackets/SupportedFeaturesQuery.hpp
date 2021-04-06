#pragma once

#include <string>
#include <set>

#include "CommandPacket.hpp"
#include "../Feature.hpp"

namespace Bloom::DebugServers::Gdb::CommandPackets
{
    using namespace Bloom::DebugServers::Gdb;

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
    private:
        std::set<Feature> supportedFeatures;

        void init();

    public:
        SupportedFeaturesQuery(std::vector<unsigned char> rawPacket): CommandPacket(rawPacket) {
            this->init();
        };

        bool isFeatureSupported(const Feature& feature) const {
            return this->supportedFeatures.find(feature) != this->supportedFeatures.end();
        }

        const std::set<Feature>& getSupportedFeatures() const {
            return this->supportedFeatures;
        }

        virtual void dispatchToHandler(Gdb::GdbRspDebugServer& gdbRspDebugServer) override;
    };
}
