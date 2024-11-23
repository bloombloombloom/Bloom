#include "DebugTranslatorConfig.hpp"

#include <cstdint>
#include <string>

#include "src/Logger/Logger.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebugSpec
{
    DebugTranslatorConfig::DebugTranslatorConfig(const YAML::Node& configNode) {
        if (configNode["targetResponseTimeout"]) {
            this->targetResponseTimeout = std::chrono::microseconds{
                configNode["targetResponseTimeout"].as<std::int64_t>(this->targetResponseTimeout.count())
            };
        }

        if (configNode["preferredMemoryAccessStrategy"]) {
            const auto strategy = configNode["preferredMemoryAccessStrategy"].as<std::string>();

            if (strategy == "abstract-command") {
                this->preferredMemoryAccessStrategy = DebugModule::MemoryAccessStrategy::ABSTRACT_COMMAND;

            } else if (strategy == "program-buffer") {
                this->preferredMemoryAccessStrategy = DebugModule::MemoryAccessStrategy::PROGRAM_BUFFER;

            } else {
                Logger::error(
                    "Invalid value (\"" + strategy + "\") provided for RISC-V debug translator config parameter "
                         "('preferredMemoryAccessStrategy'). Parameter will be ignored."
                );
            }
        }
    }
}
