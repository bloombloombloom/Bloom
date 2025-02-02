#pragma once

#include <chrono>
#include <yaml-cpp/yaml.h>
#include <optional>

#include "DebugModule/DebugModule.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebug
{
    struct DebugTranslatorConfig
    {
    public:
        /**
         * Microsecond timeout for the RISC-V target to action or respond to a command.
         */
        std::chrono::microseconds targetResponseTimeout = std::chrono::microseconds{100};

        std::optional<DebugModule::MemoryAccessStrategy> preferredMemoryAccessStrategy = std::nullopt;

        DebugTranslatorConfig() = default;
        explicit DebugTranslatorConfig(const YAML::Node& configNode);
    };
}
