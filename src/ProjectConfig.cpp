#include "ProjectConfig.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/InvalidConfig.hpp"

namespace Bloom
{
    ProjectConfig::ProjectConfig(const QJsonObject& jsonObject) {
        if (!jsonObject.contains("environments")) {
            throw Exceptions::InvalidConfig(
                "No environments found - please review the bloom.json configuration file and ensure that "
                "no syntax errors are present."
            );
        }

        // Extract all environment objects from JSON config.
        auto environments = jsonObject.find("environments").value().toObject();
        for (auto environmentIt = environments.begin(); environmentIt != environments.end(); environmentIt++) {
            auto environmentName = environmentIt.key().toStdString();

            try {
                this->environments.insert(
                    std::pair(
                        environmentName,
                        EnvironmentConfig(environmentName, environmentIt.value().toObject())
                    )
                );

            } catch (Exceptions::InvalidConfig& exception) {
                Logger::error("Invalid environment config for environment '" + environmentName + "': "
                    + exception.getMessage() + " Environment will be ignored.");
            }
        }

        if (jsonObject.contains("debugServer")) {
            this->debugServerConfig = DebugServerConfig(jsonObject.find("debugServer")->toObject());
        }

        if (jsonObject.contains("insight")) {
            this->insightConfig = InsightConfig(jsonObject.find("insight")->toObject());
        }

        if (jsonObject.contains("debugLoggingEnabled")) {
            this->debugLoggingEnabled = jsonObject.find("debugLoggingEnabled").value().toBool();
        }
    }

    InsightConfig::InsightConfig(const QJsonObject& jsonObject) {
        if (jsonObject.contains("enabled")) {
            this->insightEnabled = jsonObject.find("enabled").value().toBool();
        }
    }

    EnvironmentConfig::EnvironmentConfig(std::string name, QJsonObject jsonObject) {
        if (!jsonObject.contains("debugTool")) {
            throw Exceptions::InvalidConfig("No debug tool configuration provided.");
        }

        if (!jsonObject.contains("target")) {
            throw Exceptions::InvalidConfig("No target configuration provided.");
        }

        this->name = std::move(name);
        this->debugToolConfig = DebugToolConfig(jsonObject.find("debugTool")->toObject());
        this->targetConfig = TargetConfig(jsonObject.find("target")->toObject());

        if (jsonObject.contains("debugServer")) {
            this->debugServerConfig = DebugServerConfig(jsonObject.find("debugServer")->toObject());
        }

        if (jsonObject.contains("insight")) {
            this->insightConfig = InsightConfig(jsonObject.find("insight")->toObject());
        }
    }

    TargetConfig::TargetConfig(const QJsonObject& jsonObject) {
        if (!jsonObject.contains("name")) {
            throw Exceptions::InvalidConfig("No target name found.");
        }

        this->name = jsonObject.find("name")->toString().toLower().toStdString();

        if (jsonObject.contains("variantName")) {
            this->variantName = jsonObject.find("variantName").value().toString().toLower().toStdString();
        }

        this->jsonObject = jsonObject;
    }

    DebugToolConfig::DebugToolConfig(const QJsonObject& jsonObject) {
        if (!jsonObject.contains("name")) {
            throw Exceptions::InvalidConfig("No debug tool name found.");
        }

        this->name = jsonObject.find("name")->toString().toLower().toStdString();

        if (jsonObject.contains("releasePostDebugSession")) {
            this->releasePostDebugSession = jsonObject.find("releasePostDebugSession").value().toBool();
        }

        this->jsonObject = jsonObject;
    }

    DebugServerConfig::DebugServerConfig(const QJsonObject& jsonObject) {
        this->name = jsonObject.find("name")->toString().toLower().toStdString();
        this->jsonObject = jsonObject;
    }
}
