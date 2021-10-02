#include "ApplicationConfig.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/InvalidConfig.hpp"

using namespace Bloom;

void ApplicationConfig::init(QJsonObject jsonObject) {
    if (!jsonObject.contains("environments")) {
        throw Exceptions::InvalidConfig("No environments found.");
    }

    // Extract all environment objects from JSON config.
    auto environments = jsonObject.find("environments").value().toObject();
    for (auto environmentIt = environments.begin(); environmentIt != environments.end(); environmentIt++) {
        auto environmentName = environmentIt.key().toStdString();

        try {
            auto environmentConfig = EnvironmentConfig();
            environmentConfig.init(environmentName, environmentIt.value().toObject());

            this->environments.insert(
                std::pair<std::string, EnvironmentConfig>(environmentName, environmentConfig)
            );
        } catch (Exceptions::InvalidConfig& exception) {
            Logger::error("Invalid environment config for environment '" + environmentName + "': "
                + exception.getMessage() + " Environment will be ignored.");
        }
    }

    if (jsonObject.contains("debugServer")) {
        auto debugServerConfig = DebugServerConfig();
        debugServerConfig.init(jsonObject.find("debugServer")->toObject());
        this->debugServerConfig = debugServerConfig;
    }

    if (jsonObject.contains("insight")) {
        this->insightConfig.init(jsonObject.find("insight")->toObject());
    }

    if (jsonObject.contains("debugLoggingEnabled")) {
        this->debugLoggingEnabled = jsonObject.find("debugLoggingEnabled").value().toBool();
    }
}

void InsightConfig::init(QJsonObject jsonObject) {
    if (jsonObject.contains("enabled")) {
        this->insightEnabled = jsonObject.find("enabled").value().toBool();
    }
}

void EnvironmentConfig::init(std::string name, QJsonObject jsonObject) {
    if (!jsonObject.contains("debugTool")) {
        throw Exceptions::InvalidConfig("No debug tool configuration provided.");
    }

    if (!jsonObject.contains("target")) {
        throw Exceptions::InvalidConfig("No target configuration provided.");
    }

    this->name = name;
    this->debugToolConfig.init(jsonObject.find("debugTool")->toObject());
    this->targetConfig.init(jsonObject.find("target")->toObject());

    if (jsonObject.contains("debugServer")) {
        auto debugServerConfig = DebugServerConfig();
        debugServerConfig.init(jsonObject.find("debugServer")->toObject());
        this->debugServerConfig = debugServerConfig;
    }

    if (jsonObject.contains("insight")) {
        auto insightConfig = InsightConfig();
        insightConfig.init(jsonObject.find("insight")->toObject());
        this->insightConfig = insightConfig;
    }
}

void TargetConfig::init(QJsonObject jsonObject) {
    if (!jsonObject.contains("name")) {
        throw Exceptions::InvalidConfig("No target name found.");
    }

    this->name = jsonObject.find("name")->toString().toLower().toStdString();

    if (jsonObject.contains("variantName")) {
        this->variantName = jsonObject.find("variantName").value().toString().toLower().toStdString();
    }

    this->jsonObject = jsonObject;
}

void DebugToolConfig::init(QJsonObject jsonObject) {
    if (!jsonObject.contains("name")) {
        throw Exceptions::InvalidConfig("No debug tool name found.");
    }

    this->name = jsonObject.find("name")->toString().toLower().toStdString();

    if (jsonObject.contains("releasePostDebugSession")) {
        this->releasePostDebugSession = jsonObject.find("releasePostDebugSession").value().toBool();
    }

    this->jsonObject = jsonObject;
}

void DebugServerConfig::init(QJsonObject jsonObject) {
    this->name = jsonObject.find("name")->toString().toLower().toStdString();
    this->jsonObject = jsonObject;
}
