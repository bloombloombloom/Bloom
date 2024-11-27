#include "DebugTranslatorConfig.hpp"

#include <cstdint>
#include <string>

#include "src/Logger/Logger.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebugSpec
{
    DebugTranslatorConfig::DebugTranslatorConfig(const YAML::Node& configNode) {
        if (configNode["target_response_timeout"]) {
            this->targetResponseTimeout = std::chrono::microseconds{
                configNode["target_response_timeout"].as<std::int64_t>(this->targetResponseTimeout.count())
            };
        }

        if (configNode["preferred_memory_access_strategy"]) {
            const auto strategy = configNode["preferred_memory_access_strategy"].as<std::string>();

            if (strategy == "abstract_command") {
                this->preferredMemoryAccessStrategy = DebugModule::MemoryAccessStrategy::ABSTRACT_COMMAND;

            } else if (strategy == "program_buffer") {
                this->preferredMemoryAccessStrategy = DebugModule::MemoryAccessStrategy::PROGRAM_BUFFER;

            } else {
                Logger::error(
                    "Invalid value (\"" + strategy + "\") provided for RISC-V debug translator config parameter "
                         "('preferred_memory_access_strategy'). Parameter will be ignored."
                );
            }
        }
    }
}
