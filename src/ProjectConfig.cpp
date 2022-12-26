#include "ProjectConfig.hpp"

#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"
#include "src/Exceptions/InvalidConfig.hpp"

namespace Bloom
{
    using Services::StringService;

    ProjectConfig::ProjectConfig(const YAML::Node& configNode) {
        if (!configNode["environments"]) {
            throw Exceptions::InvalidConfig(
                "No environments found - please review the bloom.yaml configuration file and ensure that "
                "no syntax errors are present."
            );
        }

        if (!configNode["environments"].IsMap()) {
            throw Exceptions::InvalidConfig(
                "Invalid environments configuration provided - 'environments' must be of mapping type."
            );
        }

        const auto& environments = configNode["environments"];

        for (auto environmentIt = environments.begin(); environmentIt != environments.end(); environmentIt++) {
            auto environmentName = std::optional<std::string>(std::nullopt);

            try {
                environmentName = environmentIt->first.as<std::string>();

                if (!StringService::isAscii(environmentName.value())) {
                    throw Exceptions::InvalidConfig(
                        "Environment name ('" + environmentName.value() + "') is not in ASCII form."
                    );
                }

                this->environments.insert(
                    std::pair(
                        environmentName.value(),
                        EnvironmentConfig(environmentName.value(), environmentIt->second)
                    )
                );

            } catch (Exceptions::InvalidConfig& exception) {
                Logger::error(
                    "Invalid environment config for environment '" + environmentName.value() + "': "
                        + exception.getMessage() + " Environment will be ignored."
                );

            } catch (YAML::BadConversion& exception) {
                Logger::error(
                    "Invalid environment name provided. Environment names must be ASCII strings. Environment will be "
                    "ignored"
                );
            }
        }

        if (configNode["debugServer"]) {
            this->debugServerConfig = DebugServerConfig(configNode["debugServer"]);
        }

        if (configNode["insight"]) {
            this->insightConfig = InsightConfig(configNode["insight"]);
        }

        if (configNode["debugLoggingEnabled"]) {
            this->debugLoggingEnabled = configNode["debugLoggingEnabled"].as<bool>(this->debugLoggingEnabled);
        }
    }

    InsightConfig::InsightConfig(const YAML::Node& insightNode) {
        if (insightNode["enabled"]) {
            this->insightEnabled = insightNode["enabled"].as<bool>(this->insightEnabled);
        }
    }

    EnvironmentConfig::EnvironmentConfig(std::string name, const YAML::Node& environmentNode)
        : name(std::move(name))
    {
        if (!environmentNode["debugTool"]) {
            throw Exceptions::InvalidConfig("Missing debug tool configuration.");
        }

        if (!environmentNode["debugTool"].IsMap()) {
            throw Exceptions::InvalidConfig(
                "Invalid debug tool configuration provided - 'debugTool' must be of mapping type."
            );
        }

        if (!environmentNode["target"]) {
            throw Exceptions::InvalidConfig("Missing target configuration.");
        }

        if (!environmentNode["target"].IsMap()) {
            throw Exceptions::InvalidConfig(
                "Invalid target configuration provided - 'target' must be of mapping type."
            );
        }

        this->debugToolConfig = DebugToolConfig(environmentNode["debugTool"]);
        this->targetConfig = TargetConfig(environmentNode["target"]);

        if (environmentNode["debugServer"]) {
            if (!environmentNode["debugServer"].IsMap()) {
                throw Exceptions::InvalidConfig(
                    "Invalid debug server configuration provided - 'debugServer' must be of mapping type."
                );
            }

            this->debugServerConfig = DebugServerConfig(environmentNode["debugServer"]);
        }

        if (environmentNode["insight"]) {
            if (!environmentNode["insight"].IsMap()) {
                throw Exceptions::InvalidConfig(
                    "Invalid insight configuration provided - 'insight' must be of mapping type."
                );
            }

            this->insightConfig = InsightConfig(environmentNode["insight"]);
        }

        if (environmentNode["shutdownPostDebugSession"]) {
            this->shutdownPostDebugSession = environmentNode["shutdownPostDebugSession"].as<bool>(
                this->shutdownPostDebugSession
            );
        }
    }

    TargetConfig::TargetConfig(const YAML::Node& targetNode) {
        if (!targetNode["name"]) {
            throw Exceptions::InvalidConfig("No target name found.");
        }

        this->name = StringService::asciiToLower(targetNode["name"].as<std::string>());

        if (targetNode["variantName"]) {
            this->variantName = StringService::asciiToLower(targetNode["variantName"].as<std::string>());
        }

        this->targetNode = targetNode;
    }

    DebugToolConfig::DebugToolConfig(const YAML::Node& debugToolNode) {
        if (!debugToolNode["name"]) {
            throw Exceptions::InvalidConfig("No debug tool name found.");
        }

        this->name = StringService::asciiToLower(debugToolNode["name"].as<std::string>());

        if (debugToolNode["releasePostDebugSession"]) {
            this->releasePostDebugSession = debugToolNode["releasePostDebugSession"].as<bool>(
                this->releasePostDebugSession
            );
        }

        this->debugToolNode = debugToolNode;
    }

    DebugServerConfig::DebugServerConfig(const YAML::Node& debugServerNode) {
        if (!debugServerNode["name"]) {
            throw Exceptions::InvalidConfig("No debug server name found.");
        }

        this->name = StringService::asciiToLower(debugServerNode["name"].as<std::string>());
        this->debugServerNode = debugServerNode;
    }
}
