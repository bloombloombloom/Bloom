#pragma once

#include <chrono>
#include <yaml-cpp/yaml.h>

namespace DebugToolDrivers::Protocols::RiscVDebugSpec
{
    struct DebugTranslatorConfig
    {
    public:
        /**
         * Microsecond timeout for the RISC-V target to action or respond to a command.
         */
        std::chrono::microseconds targetResponseTimeout = std::chrono::microseconds{100};

        DebugTranslatorConfig() = default;
        explicit DebugTranslatorConfig(const YAML::Node& configNode);
    };
}
