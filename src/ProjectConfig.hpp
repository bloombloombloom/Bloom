#pragma once

#include <memory>
#include <map>
#include <string>
#include <QJsonObject>

namespace Bloom
{
    /*
     * Currently, all user configuration is stored in a JSON file (bloom.json), in the user's project directory.
     *
     * The JSON config file should define debugging environment objects. A debugging environment object is just
     * a user defined JSON object that holds parameters relating to a specific debugging environment (e.g config params
     * for the DebugTool, Target configuration and any debug server config). Because a config file
     * can define multiple debugging environments, each object should be assigned a key in the config file. We use this
     * key to allow users to select different debugging environments between debugging sessions.
     *
     * On application startup, we extract the config from this JSON file and generate an ProjectConfig object.
     * See Application::extractConfig() for more on this.
     *
     * Some config parameters are specific to certain entities within Bloom, but have no significance across the
     * rest of the application. For example, AVR8 targets require 'physicalInterface' and 'configVariant' parameters.
     * These are used to configure the AVR8 target, but have no significance across the rest of the application.
     * This is why some configuration structs (like TargetConfig) include a QJsonObject member, typically named jsonObject.
     * When instances of these structs are passed to the appropriate entities, any configuration required by those
     * entities is extracted from the jsonObject member. This means we don't have to worry about any entity specific
     * config parameters at the application level. We can simply extract what we need at an entity level and the rest
     * of the application can remain oblivious. For an example on extracting entity specific config, see
     * AVR8::preActivationConfigure().
     *
     * For more on project configuration, see Bloom documentation at https://bloom.oscillate.io/docs/configuration
     */

    /**
     * Configuration relating to a specific target.
     *
     * Please don't define any target specific configuration here, unless it applies to *all* targets across
     * the application. If a target requires specific config, it should be extracted from the jsonObject member.
     * This should be done in Target::preActivationConfigure(), to which an instance of TargetConfig is passed.
     * See the comment above on entity specific config for more on this.
     */
    struct TargetConfig
    {
        /**
         * Obtains config parameters from JSON object.
         *
         * @param jsonObject
         */
        void init(const QJsonObject& jsonObject);

        /**
         * The name of the selected target.
         */
        std::string name;

        /**
         * The name of the selected target variant.
         *
         * Insight uses this to determine which variant to select on startup.
         */
        std::optional<std::string> variantName;

        QJsonObject jsonObject;
    };

    /**
     * Configuration relating to a specific debug tool.
     *
     * As with the TargetConfig struct, please don't add any manufacture/model specific configuration here. This
     * configuration should apply to all supported debug tools. Specific configuration can be extracted from the
     * jsonObject member, as described in the TargetConfig comment above.
     */
    struct DebugToolConfig
    {
        /**
         * Obtains config parameters from JSON object.
         *
         * @param jsonObject
         */
        void init(const QJsonObject& jsonObject);

        /**
         * The name of the selected debug tool.
         */
        std::string name;

        /**
         * Determines if the TargetController will release the debug tool at the end of a debug session.
         *
         * If this is enabled, the TargetController will automatically suspend once the current debug session has
         * ended. If not enabled, the TargetController will remain active and in control of the debug tool, preventing
         * the user from running any other application that needs access to the debug tool.
         */
        bool releasePostDebugSession = true;

        QJsonObject jsonObject;
    };

    /**
     * Debug server configuration.
     */
    struct DebugServerConfig
    {
        /**
         * Obtains config parameters from JSON object.
         *
         * @param jsonObject
         */
        void init(const QJsonObject& jsonObject);

        std::string name;
        QJsonObject jsonObject;
    };

    struct InsightConfig
    {
        /**
         * Obtains config parameters from JSON object.
         *
         * @param jsonObject
         */
        void init(const QJsonObject& jsonObject);

        bool insightEnabled = true;
    };

    /**
     * Configuration relating to a specific user defined environment.
     *
     * An instance of this type will be instantiated for each environment defined in the user's config file.
     * See Application::extractConfig() implementation for more on this.
     */
    struct EnvironmentConfig
    {
        /**
         * Obtains config parameters from JSON object.
         *
         * @param jsonObject
         */
        void init(std::string name, QJsonObject jsonObject);

        /**
         * The environment name is stored as the key to the JSON object containing the environment parameters.
         *
         * Environment names must be unique.
         */
        std::string name;

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
    };

    /**
     * This holds all user provided project configuration.
     */
    struct ProjectConfig
    {
        /**
         * Obtains config parameters from JSON object.
         *
         * @param jsonObject
         */
        void init(const QJsonObject& jsonObject);

        /**
         * A mapping of environment names to EnvironmentConfig objects.
         */
        std::map<std::string, EnvironmentConfig> environments;

        /**
         * Application level debug server configuration. We use this as a fallback if no debug server config is
         * provided at the environment level.
         */
        std::optional<DebugServerConfig> debugServerConfig;

        InsightConfig insightConfig;

        bool debugLoggingEnabled = false;
    };
}
