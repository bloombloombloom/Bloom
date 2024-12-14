#include "WchRiscVTargetConfig.hpp"

namespace Targets::RiscV
{
    WchRiscVTargetConfig::WchRiscVTargetConfig(const RiscVTargetConfig& targetConfig)
        : RiscVTargetConfig(targetConfig)
    {
        const auto& targetNode = targetConfig.targetNode;

        if (targetNode["program_segment_key"]) {
            this->programSegmentKey = targetNode["program_segment_key"].as<std::string>("");
        }
    }
}
