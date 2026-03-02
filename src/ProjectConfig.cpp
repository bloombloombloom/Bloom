#include "ProjectConfig.hpp"

#include "src/Services/StringService.hpp"
#include "src/Services/PathService.hpp"
#include "src/Logger/Logger.hpp"
#include "src/Exceptions/InvalidConfig.hpp"

using Services::StringService;

ProjectConfig::ProjectConfig(const YAML::Node& configNode) {
    if (!configNode["environments"]) {
        throw Exceptions::InvalidConfig{
            "No environments found - please review the bloom.yaml configuration file and ensure that "
               "no syntax errors are present."
        };
    }

    if (!configNode["environments"].IsMap()) {
        throw Exceptions::InvalidConfig{
            "Invalid environments configuration provided - 'environments' must be of mapping type."
        };
    }

    const auto& environments = configNode["environments"];

    for (auto environmentIt = environments.begin(); environmentIt != environments.end(); environmentIt++) {
        auto environmentName = std::optional<std::string>{};

        try {
            environmentName = environmentIt->first.as<std::string>();

            if (!StringService::isAscii(environmentName.value())) {
                throw Exceptions::InvalidConfig{
                    "Environment name ('" + environmentName.value() + "') is not in ASCII form."
                };
            }

            this->environments.emplace(
                environmentName.value(),
                EnvironmentConfig{environmentName.value(), environmentIt->second}
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
        Logger::warning(
            "The 'debugServer' key was renamed to 'server' in v1.0.0. Please update your bloom.yaml configuration. "
                "See " + Services::PathService::homeDomainName() + "/docs/v1-0-0-migration for more."
        );
    }

    if (configNode["server"]) {
        this->debugServerConfig = DebugServerConfig(configNode["server"]);
    }

    if (configNode["insight"]) {
        this->insightConfig = InsightConfig(configNode["insight"]);
    }

    // Old param name, will remove later
    if (configNode["debugLoggingEnabled"]) {
        this->debugLogging = configNode["debugLoggingEnabled"].as<bool>(this->debugLogging);
    }

    if (configNode["debug_logging"]) {
        this->debugLogging = configNode["debug_logging"].as<bool>(this->debugLogging);
    }
}

InsightConfig::InsightConfig(const YAML::Node& insightNode) {
    if (!insightNode.IsMap()) {
        throw Exceptions::InvalidConfig{
            "Invalid insight configuration provided - node must take the form of a YAML mapping."
        };
    }

    if (insightNode["activate_on_startup"]) {
        this->activateOnStartup = insightNode["activate_on_startup"].as<bool>(this->activateOnStartup);
    }

    if (insightNode["shutdown_on_close"]) {
        this->shutdownOnClose = insightNode["shutdown_on_close"].as<bool>(this->shutdownOnClose);
    }

    if (insightNode["default_variant_key"]) {
        this->defaultVariantKey = insightNode["default_variant_key"].as<std::string>();
    }
}

EnvironmentConfig::EnvironmentConfig(std::string name, const YAML::Node& environmentNode)
    : name(std::move(name))
{
    if (!environmentNode.IsMap()) {
        throw Exceptions::InvalidConfig{"Environment node must take the form of a YAML mapping."};
    }

    static auto warn = true;

    if (warn) {
        if (environmentNode["debugTool"] && !environmentNode["tool"]) {
            Logger::warning(
                "The 'debugTool' key was renamed to 'tool' in v1.0.0. Please update your bloom.yaml configuration. "
                    "Bloom will fail to start up until this is resolved. See "
                    + Services::PathService::homeDomainName() + "/docs/v1-0-0-migration for more."
            );
        }

        if (environmentNode["debugServer"] && !environmentNode["server"]) {
            Logger::warning(
                "The 'debugServer' key was renamed to 'server' in v1.0.0. Please update your bloom.yaml configuration. "
                    "Bloom will fail to start up until this is resolved. See "
                    + Services::PathService::homeDomainName() + "/docs/v1-0-0-migration for more."
            );
        }

        warn = false;
    }

    if (!environmentNode["tool"]) {
        throw Exceptions::InvalidConfig{"Missing debug tool configuration."};
    }

    if (!environmentNode["target"]) {
        throw Exceptions::InvalidConfig{"Missing target configuration."};
    }

    this->debugToolConfig = DebugToolConfig{environmentNode["tool"]};
    this->targetConfig = TargetConfig{environmentNode["target"]};

    if (environmentNode["server"]) {
        this->debugServerConfig = DebugServerConfig{environmentNode["server"]};
    }

    if (environmentNode["insight"]) {
        this->insightConfig = InsightConfig(environmentNode["insight"]);
    }

    if (environmentNode["shutdown_post_debug_session"]) {
        this->shutdownPostDebugSession = environmentNode["shutdown_post_debug_session"].as<bool>(
            this->shutdownPostDebugSession
        );
    }
}

TargetConfig::TargetConfig(const YAML::Node& targetNode) {
    using Targets::TargetPhysicalInterface;

    if (!targetNode.IsMap()) {
        throw Exceptions::InvalidConfig{
            "Invalid target configuration provided - node must take the form of a YAML mapping."
        };
    }

    if (!targetNode["name"]) {
        throw Exceptions::InvalidConfig{"No target name found."};
    }

    this->name = StringService::asciiToLower(targetNode["name"].as<std::string>());

    static auto physicalInterfacesByConfigName = std::map<std::string, TargetPhysicalInterface>{
        {"debug_wire", TargetPhysicalInterface::DEBUG_WIRE},
        {"pdi", TargetPhysicalInterface::PDI},
        {"jtag", TargetPhysicalInterface::JTAG},
        {"updi", TargetPhysicalInterface::UPDI},
        {"sdi", TargetPhysicalInterface::SDI},
    };

    if (!targetNode["physical_interface"]) {
        throw Exceptions::InvalidConfig{"No physical interface specified."};
    }

    const auto physicalInterfaceName = StringService::asciiToLower(targetNode["physical_interface"].as<std::string>());
    const auto physicalInterfaceIt = physicalInterfacesByConfigName.find(physicalInterfaceName);

    if (physicalInterfaceIt == physicalInterfacesByConfigName.end()) {
        throw Exceptions::InvalidConfig{
            "Invalid physical interface provided (\"" + physicalInterfaceName + "\") for target. "
                "See " + Services::PathService::homeDomainName() + "/docs/configuration/target-physical-interfaces "
                "for valid physical interface configuration values."
        };
    }

    this->physicalInterface = physicalInterfaceIt->second;

    if (targetNode["resume_on_startup"]) {
        this->resumeOnStartup = targetNode["resume_on_startup"].as<bool>(this->resumeOnStartup);
    }

    if (targetNode["variantName"]) {
        Logger::warning(
            "The 'variantName' target configuration parameter was removed in v2.0.0. Please use the "
                "'default_variant_key' insight configuration parameter."
        );
    }

    if (targetNode["hardware_breakpoints"]) {
        this->hardwareBreakpoints = targetNode["hardware_breakpoints"].as<bool>(this->hardwareBreakpoints);
    }

    if (targetNode["program_memory_cache"]) {
        this->programMemoryCache = targetNode["program_memory_cache"].as<bool>(this->programMemoryCache);
    }

    if (targetNode["delta_programming"]) {
        this->deltaProgramming = targetNode["delta_programming"].as<bool>(this->deltaProgramming);
    }

    if (targetNode["reserve_stepping_breakpoint"]) {
        this->reserveSteppingBreakpoint = targetNode["reserve_stepping_breakpoint"].as<bool>(false);
    }

    this->targetNode = targetNode;
}

DebugToolConfig::DebugToolConfig(const YAML::Node& toolNode) {
    if (!toolNode.IsMap()) {
        throw Exceptions::InvalidConfig{
            "Invalid debug tool configuration provided - node must take the form of a YAML mapping."
        };
    }

    if (!toolNode["name"]) {
        throw Exceptions::InvalidConfig{"No debug tool name found."};
    }

    this->name = StringService::asciiToLower(toolNode["name"].as<std::string>());
    this->toolNode = toolNode;
}

DebugServerConfig::DebugServerConfig(const YAML::Node& debugServerNode) {
    if (!debugServerNode.IsMap()) {
        throw Exceptions::InvalidConfig{
            "Invalid debug server configuration provided - node must take the form of a YAML mapping."
        };
    }

    if (!debugServerNode["name"]) {
        throw Exceptions::InvalidConfig{"No debug server name found."};
    }

    this->name = StringService::asciiToLower(debugServerNode["name"].as<std::string>());
    this->debugServerNode = debugServerNode;
}
