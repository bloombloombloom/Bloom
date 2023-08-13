#include "SupportedFeaturesResponse.hpp"

namespace DebugServer::Gdb::ResponsePackets
{
    SupportedFeaturesResponse::SupportedFeaturesResponse(
        const std::set<std::pair<Feature, std::optional<std::string>>>& supportedFeatures
    )
        : supportedFeatures(supportedFeatures)
    {
        auto output = std::string("qSupported:");
        static const auto gdbFeatureMapping = getGdbFeatureToNameMapping();

        for (const auto& supportedFeature : this->supportedFeatures) {
            const auto featureString = gdbFeatureMapping.valueAt(supportedFeature.first);

            if (featureString.has_value()) {
                if (supportedFeature.second.has_value()) {
                    output.append(featureString.value() + "=" + supportedFeature.second.value() + ";");

                } else {
                    output.append(featureString.value() + "+;");
                }

            }
        }

        this->data = {output.begin(), output.end()};
    }
}
