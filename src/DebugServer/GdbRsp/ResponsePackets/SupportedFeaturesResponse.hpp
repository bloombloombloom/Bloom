#pragma once

#include <set>
#include <utility>

#include "ResponsePacket.hpp"
#include "../Feature.hpp"

namespace Bloom::DebugServer::Gdb::ResponsePackets
{
    /**
     * The SupportedFeaturesResponse class implements the response packet structure for the "qSupported" command.
     */
    class SupportedFeaturesResponse: public ResponsePacket
    {
    public:
        SupportedFeaturesResponse() = default;
        explicit SupportedFeaturesResponse(std::set<std::pair<Feature, std::optional<std::string>>> supportedFeatures)
        : supportedFeatures(std::move(supportedFeatures)) {};

        [[nodiscard]] std::vector<unsigned char> getData() const override;

    protected:
        std::set<std::pair<Feature, std::optional<std::string>>> supportedFeatures;
    };
}
