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
 * On application start up, we extract the config from this YAML file and construct a ProjectConfig object.
 * See Application::loadProjectConfiguration() for more on this.
 *
 * For more on project configuration, see Bloom documentation at https://bloom.oscillate.io/docs/configuration
 */

/**
 * Configuration relating to a specific target.
 *
 * Please don't define any target specific configuration here, unless it applies to *all* targets across
 * the application. If a target requires specific config, it should be extracted from the YAML::Node (targetNode)
 * member.
 */
struct TargetConfig
{
    std::string name;
    Targets::TargetPhysicalInterface physicalInterface;
    bool resumeOnStartup = false;
    bool hardwareBreakpoints = true;
    bool programMemoryCache = true;
    bool deltaProgramming = true;
    std::optional<bool> reserveSteppingBreakpoint = std::nullopt;

    YAML::Node targetNode;

    TargetConfig() = default;
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
    std::string name;

    YAML::Node toolNode;

    DebugToolConfig() = default;
    explicit DebugToolConfig(const YAML::Node& toolNode);
};

/**
 * Debug server configuration.
 */
struct DebugServerConfig
{
    std::string name;

    YAML::Node debugServerNode;

    DebugServerConfig() = default;
    explicit DebugServerConfig(const YAML::Node& debugServerNode);
};

struct InsightConfig
{
    bool activateOnStartup = false;
    bool shutdownOnClose = false;
    std::optional<std::string> defaultVariantKey = std::nullopt;

    InsightConfig() = default;
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
    std::string name;
    bool shutdownPostDebugSession = false;

    /**
     * CLion expects the debug server (in this case, Bloom) to shut down immediately upon receiving the detach command
     * via GDB. It does not allow for any time to shut down after that - it just kills the process with a SIGKILL.
     *
     * I raised this with JetBrains, a number of years ago: https://youtrack.jetbrains.com/issue/CPP-28843
     *
     * They didn't seem to have any interest in fixing the issue, so I had to include some CLion-specific behaviour
     * in Bloom, to prevent CLion from killing Bloom's process before we got a chance to shut down cleanly. It was a
     * hacky solution, but it worked.
     *
     * Recently (in version 2024.3), CLion introduced "debug servers", which allows Bloom to remain running in the
     * background, between debug sessions, reducing the time it takes to stop and start new debug sessions.
     *
     * This doesn't fix the issue described above - CLion still behaves like an aggressive dickhead, sending SIGKILL
     * signals whenever it likes. However, Bloom's CLion-specific functionality may conflict with this new "debug server"
     * functionality, so it's necessary to allow the user to disable it in their project config.
     */
    bool clionAdaptation = true;

    DebugToolConfig debugToolConfig;
    TargetConfig targetConfig;
    std::optional<DebugServerConfig> debugServerConfig;
    std::optional<InsightConfig> insightConfig;

    EnvironmentConfig(std::string name, const YAML::Node& environmentNode);
};

/**
 * This holds all user provided project configuration.
 */
struct ProjectConfig
{
    std::map<std::string, EnvironmentConfig> environments;
    std::optional<DebugServerConfig> debugServerConfig;
    InsightConfig insightConfig = {};
    bool debugLogging = false;

    explicit ProjectConfig(const YAML::Node& configNode);
};
