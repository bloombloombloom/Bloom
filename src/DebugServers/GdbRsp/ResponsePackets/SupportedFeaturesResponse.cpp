#include "SupportedFeaturesResponse.hpp"

namespace Bloom::DebugServer::Gdb::ResponsePackets
{
    std::vector<unsigned char> SupportedFeaturesResponse::getData() const {
        std::string output = "qSupported:";
        auto gdbFeatureMapping = getGdbFeatureToNameMapping();

        for (const auto& supportedFeature : this->supportedFeatures) {
            auto featureString = gdbFeatureMapping.valueAt(supportedFeature.first);

            if (featureString.has_value()) {
                if (supportedFeature.second.has_value()) {
                    output.append(featureString.value() + "=" + supportedFeature.second.value() + ";");

                } else {
                    output.append(featureString.value() + "+;");
                }

            }
        }

        return std::vector<unsigned char>(output.begin(), output.end());
    }
}
