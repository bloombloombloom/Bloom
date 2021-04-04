#pragma once

#include <set>

#include "ResponsePacket.hpp"
#include "../Feature.hpp"

namespace Bloom::DebugServers::Gdb::ResponsePackets
{
    /**
     * The SupportedFeaturesResponse class implements the response packet structure for the "qSupported" command.
     */
    class SupportedFeaturesResponse: public ResponsePacket
    {
    protected:
        std::set<std::pair<Feature, std::optional<std::string>>> supportedFeatures;

    public:
        SupportedFeaturesResponse() = default;
        SupportedFeaturesResponse(const std::set<std::pair<Feature, std::optional<std::string>>>& supportedFeatures)
        : supportedFeatures(supportedFeatures) {};

        std::vector<unsigned char> getData() const override;
    };
}
