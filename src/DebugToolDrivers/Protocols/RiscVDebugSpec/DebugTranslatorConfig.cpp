#include "DebugTranslatorConfig.hpp"

#include <cstdint>

namespace DebugToolDrivers::Protocols::RiscVDebugSpec
{
    DebugTranslatorConfig::DebugTranslatorConfig(const YAML::Node& configNode) {
        if (configNode["targetResponseTimeout"]) {
            this->targetResponseTimeout = std::chrono::microseconds{
                configNode["targetResponseTimeout"].as<std::int64_t>(this->targetResponseTimeout.count())
            };
        }
    }
}
