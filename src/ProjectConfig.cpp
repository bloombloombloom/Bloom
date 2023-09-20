#include "ProjectConfig.hpp"

#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"
#include "src/Exceptions/InvalidConfig.hpp"

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

    if (configNode["server"]) {
        this->debugServerConfig = DebugServerConfig(configNode["server"]);
    }

    if (configNode["insight"]) {
        this->insightConfig = InsightConfig(configNode["insight"]);
    }

    if (configNode["debugLoggingEnabled"]) {
        this->debugLoggingEnabled = configNode["debugLoggingEnabled"].as<bool>(this->debugLoggingEnabled);
    }
}

InsightConfig::InsightConfig(const YAML::Node& insightNode) {
    if (!insightNode.IsMap()) {
        throw Exceptions::InvalidConfig(
            "Invalid insight configuration provided - node must take the form of a YAML mapping."
        );
    }

    if (insightNode["activateOnStartup"]) {
        this->activateOnStartup = insightNode["activateOnStartup"].as<bool>(this->activateOnStartup);
    }

    if (insightNode["shutdownOnClose"]) {
        this->shutdownOnClose = insightNode["shutdownOnClose"].as<bool>(this->shutdownOnClose);
    }
}

EnvironmentConfig::EnvironmentConfig(std::string name, const YAML::Node& environmentNode)
    : name(std::move(name))
{
    if (!environmentNode.IsMap()) {
        throw Exceptions::InvalidConfig("Environment node must take the form of a YAML mapping.");
    }

    if (!environmentNode["tool"]) {
        throw Exceptions::InvalidConfig("Missing debug tool configuration.");
    }

    if (!environmentNode["target"]) {
        throw Exceptions::InvalidConfig("Missing target configuration.");
    }

    this->debugToolConfig = DebugToolConfig(environmentNode["tool"]);
    this->targetConfig = TargetConfig(environmentNode["target"]);

    if (environmentNode["server"]) {
        this->debugServerConfig = DebugServerConfig(environmentNode["server"]);
    }

    if (environmentNode["insight"]) {
        this->insightConfig = InsightConfig(environmentNode["insight"]);
    }

    if (environmentNode["shutdownPostDebugSession"]) {
        this->shutdownPostDebugSession = environmentNode["shutdownPostDebugSession"].as<bool>(
            this->shutdownPostDebugSession
        );
    }
}

TargetConfig::TargetConfig(const YAML::Node& targetNode) {
    if (!targetNode.IsMap()) {
        throw Exceptions::InvalidConfig(
            "Invalid target configuration provided - node must take the form of a YAML mapping."
        );
    }

    if (!targetNode["name"]) {
        throw Exceptions::InvalidConfig("No target name found.");
    }

    this->name = StringService::asciiToLower(targetNode["name"].as<std::string>());

    if (targetNode["variantName"]) {
        this->variantName = StringService::asciiToLower(targetNode["variantName"].as<std::string>());
    }

    if (targetNode["hardwareBreakpoints"]) {
        this->hardwareBreakpoints = targetNode["hardwareBreakpoints"].as<bool>(this->hardwareBreakpoints);
    }

    this->targetNode = targetNode;
}

DebugToolConfig::DebugToolConfig(const YAML::Node& debugToolNode) {
    if (!debugToolNode.IsMap()) {
        throw Exceptions::InvalidConfig(
            "Invalid debug tool configuration provided - node must take the form of a YAML mapping."
        );
    }

    if (!debugToolNode["name"]) {
        throw Exceptions::InvalidConfig("No debug tool name found.");
    }

    this->name = StringService::asciiToLower(debugToolNode["name"].as<std::string>());
    this->debugToolNode = debugToolNode;
}

DebugServerConfig::DebugServerConfig(const YAML::Node& debugServerNode) {
    if (!debugServerNode.IsMap()) {
        throw Exceptions::InvalidConfig(
            "Invalid debug server configuration provided - node must take the form of a YAML mapping."
        );
    }

    if (!debugServerNode["name"]) {
        throw Exceptions::InvalidConfig("No debug server name found.");
    }

    this->name = StringService::asciiToLower(debugServerNode["name"].as<std::string>());
    this->debugServerNode = debugServerNode;
}
