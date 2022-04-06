#pragma once

#include <set>
#include <utility>

#include "ResponsePacket.hpp"

#include "src/DebugServer/Gdb/Feature.hpp"

namespace Bloom::DebugServer::Gdb::ResponsePackets
{
    /**
     * The SupportedFeaturesResponse class implements the response packet structure for the "qSupported" command.
     */
    class SupportedFeaturesResponse: public ResponsePacket
    {
    public:
        explicit SupportedFeaturesResponse(
            const std::set<std::pair<Feature, std::optional<std::string>>>& supportedFeatures
        );

    private:
        std::set<std::pair<Feature, std::optional<std::string>>> supportedFeatures;
    };
}
