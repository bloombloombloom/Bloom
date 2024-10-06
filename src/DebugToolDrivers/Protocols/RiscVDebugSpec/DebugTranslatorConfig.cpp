#include "DebugTranslatorConfig.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebugSpec
{
    DebugTranslatorConfig::DebugTranslatorConfig(const YAML::Node& configNode) {
        if (configNode["targetResponseTimeout"]) {
            this->targetResponseTimeout = std::chrono::microseconds{
                configNode["targetResponseTimeout"].as<int>(this->targetResponseTimeout.count())
            };
        }
    }
}
