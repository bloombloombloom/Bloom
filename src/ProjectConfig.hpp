#pragma once

#include <memory>
#include <map>
#include <string>
#include <optional>
#include <yaml-cpp/yaml.h>

#include "src/Targets/TargetPhysicalInterface.hpp"

/*
 * Currently, all user configuration is stored in a YAML file (bloom.yaml), in the user's project directory.
 *
 * The YAML config file should define parameters for specific debug environments. Because a config file can define
 * multiple debug environments, each environment must be assigned a unique name that can be used to identify the
 * environment. This is why the 'environments' parameter is a YAML map, with the key being the environment name.
 *
 * On application start up, we extract the config from this YAML file and generate a ProjectConfig object.
 * See Application::loadProjectConfiguration() for more on this.
 *
 * Some config parameters are specific to certain entities within Bloom, but have no significance across the
 * rest of the application. For example, AVR8 targets require the 'physicalInterface' and other AVR8 specific
 * parameters. These are used to configure AVR8 targets, but have no significance across the rest of the
 * application. This is why some configuration structs (like TargetConfig) include a YAML::Node member.
 * When instances of these structs are passed to the appropriate entities, any configuration required by those
 * entities is extracted from the YAML::Node member. This means we don't have to worry about any entity specific
 * config parameters at the application level. We can simply extract what we need at an entity level and the rest
 * of the application can remain oblivious. For an example on extracting entity specific config, see
 * Avr8TargetConfig::Avr8TargetConfig() and Avr8::preActivationConfigure().
 *
 * For more on project configuration, see Bloom documentation at https://bloom.oscillate.io/docs/configuration
 */

/**
 * Configuration relating to a specific target.
 *
 * Please don't define any target specific configuration here, unless it applies to *all* targets across
 * the application. If a target requires specific config, it should be extracted from the YAML::Node (targetNode)
 * member. This should be done in Target::preActivationConfigure(), to which an instance of TargetConfig is passed.
 * See the comment above on entity specific config for more on this.
 */
struct TargetConfig
{
    /**
     * The name of the selected target.
     */
    std::string name;

    /**
     * The physical interface is the interface used for communication between the debug tool and the connected
     * target.
     */
    Targets::TargetPhysicalInterface physicalInterface;

    /**
     * The name of the selected target variant.
     *
     * Insight uses this to determine which variant to select on start up.
     */
    std::optional<std::string> variantName;

    /**
     * Determines whether Bloom will make use of the target's hardware breakpoint resources (if available).
     */
    bool hardwareBreakpoints = true;

    /**
     * Determines whether Bloom will employ a cache for the target's program memory.
     */
    bool programMemoryCache = true;

    /**
     * For extracting any target specific configuration. See Avr8TargetConfig::Avr8TargetConfig() and
     * Avr8::preActivationConfigure() for an example of this.
     */
    YAML::Node targetNode;

    TargetConfig() = default;

    /**
     * Obtains config parameters from YAML node.
     *
     * @param targetNode
     */
    explicit TargetConfig(const YAML::Node& targetNode);
};

/**
 * Configuration relating to a specific debug tool.
 *
 * As with the TargetConfig struct, please don't add any manufacture/model specific configuration here. This
 * configuration should apply to all supported debug tools. Specific configuration can be extracted from the
 * YAML::Node (debugToolNode) member, as described in the TargetConfig comment above.
 */
struct DebugToolConfig
{
    /**
     * The name of the selected debug tool.
     */
    std::string name;

    /**
     * For extracting any debug tool specific configuration.
     */
    YAML::Node debugToolNode;

    DebugToolConfig() = default;

    /**
     * Obtains config parameters from YAML node.
     *
     * @param debugToolNode
     */
    explicit DebugToolConfig(const YAML::Node& debugToolNode);
};

/**
 * Debug server configuration.
 */
struct DebugServerConfig
{
    /**
     * The name of the selected debug server.
     */
    std::string name;

    /**
     * For extracting any debug server specific configuration. See GdbDebugServerConfig::GdbDebugServerConfig() and
     * GdbRspDebugServer::GdbRspDebugServer() for an example of this.
     */
    YAML::Node debugServerNode;

    DebugServerConfig() = default;

    /**
     * Obtains config parameters from YAML node.
     *
     * @param debugServerNode
     */
    explicit DebugServerConfig(const YAML::Node& debugServerNode);
};

struct InsightConfig
{
    /**
     * If true, the Insight GUI will be activated immediately at startup.
     */
    bool activateOnStartup = false;

    /**
     * If true, Bloom will shutdown when the user closes the Insight GUI.
     */
    bool shutdownOnClose = false;

    InsightConfig() = default;

    /**
     * Obtains config parameters from YAML node.
     *
     * @param insightNode
     */
    explicit InsightConfig(const YAML::Node& insightNode);
};

/**
 * Configuration relating to a specific user defined environment.
 *
 * An instance of this type will be instantiated for each environment defined in the user's config file.
 * See Application::loadProjectConfiguration() implementation for more on this.
 */
struct EnvironmentConfig
{
    /**
     * The environment name is stored as the key to the YAML map containing the environment parameters.
     *
     * Environment names must be unique.
     */
    std::string name;

    /**
     * Flag to determine whether Bloom should shutdown at the end of a debug session.
     */
    bool shutdownPostDebugSession = false;

    /**
     * Configuration for the environment's selected debug tool.
     *
     * Each environment can select only one debug tool.
     */
    DebugToolConfig debugToolConfig;

    /**
     * Configuration for the environment's selected target.
     *
     * Each environment can select only one target.
     */
    TargetConfig targetConfig;

    /**
     * Configuration for the environment's debug server. Users can define this at the application level if
     * they desire.
     */
    std::optional<DebugServerConfig> debugServerConfig;

    /**
     * Insight configuration can be defined at an environment level as well as at an application level.
     */
    std::optional<InsightConfig> insightConfig;

    /**
     * Obtains config parameters from YAML node.
     *
     * @param name
     * @param environmentNode
     */
    EnvironmentConfig(std::string name, const YAML::Node& environmentNode);
};

/**
 * This holds all user provided project configuration.
 */
struct ProjectConfig
{
    /**
     * A mapping of environment names to EnvironmentConfig objects.
     */
    std::map<std::string, EnvironmentConfig> environments;

    /**
     * Application level debug server configuration. We use this as a fallback if no debug server config is
     * provided at the environment level.
     */
    std::optional<DebugServerConfig> debugServerConfig;

    /**
     * Application level Insight configuration. We use this as a fallback if no Insight config is provided at
     * the environment level.
     *
     * We don't use std::optional here because the InsightConfig has no mandatory parameters, so users may wish to
     * omit the 'insight' node from their bloom.yaml file, entirely. In this case, Bloom should fall back to a default
     * constructed, project-level, InsightConfig instance.
     */
    InsightConfig insightConfig = {};

    bool debugLogging = false;

    /**
     * Obtains config parameters from YAML node.
     *
     * @param configNode
     */
    explicit ProjectConfig(const YAML::Node& configNode);
};
