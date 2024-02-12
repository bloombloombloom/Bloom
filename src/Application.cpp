#include "Application.hpp"

#include <iostream>
#include <QTimer>
#include <QFile>
#include <QJsonDocument>
#include <unistd.h>
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/exceptions.h>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QUrl>
#include <QUrlQuery>

#include "src/Logger/Logger.hpp"
#include "src/Services/PathService.hpp"
#include "src/Services/ProcessService.hpp"
#include "src/Helpers/BiMap.hpp"

#include "src/Targets/TargetDescription/TargetDescriptionFile.hpp"

#include "src/Exceptions/InvalidConfig.hpp"

using namespace Exceptions;

Application::Application(std::vector<std::string>&& arguments)
    : arguments(std::move(arguments))
    , qtApplication(
        (
            Thread::blockAllSignals(),
            QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true),
#ifndef BLOOM_DEBUG_BUILD
            QCoreApplication::addLibraryPath(QString::fromStdString(Services::PathService::applicationDirPath() + "/plugins")),
#endif
#ifndef EXCLUDE_INSIGHT
            QApplication(this->qtApplicationArgc, this->qtApplicationArgv.data())
#else
            QCoreApplication(this->qtApplicationArgc, this->qtApplicationArgv.data())
#endif
        )
    )
{}

int Application::run() {
    try {
        this->setName("Bloom");

        if (this->arguments.size() > 1) {
            auto& firstArg = this->arguments.at(1);
            const auto commandHandlersByCommandName = this->getCommandHandlersByCommandName();
            const auto commandHandlerIt = commandHandlersByCommandName.find(firstArg);

            if (commandHandlerIt != commandHandlersByCommandName.end()) {
                // User has passed an argument that maps to a command callback - invoke the callback and shutdown
                const auto returnValue = commandHandlerIt->second();

                this->shutdown();
                return returnValue;
            }

            // If the first argument didn't map to a command, we assume it's an environment name
            this->selectedEnvironmentName = std::move(firstArg);
        }

        if (Services::ProcessService::isRunningAsRoot()) {
            Logger::warning("Please don't run Bloom as root - you're asking for trouble.");
        }

#ifdef BLOOM_DEBUG_BUILD
        Logger::warning("This is a debug build - some functions may not work as expected");
#endif

#ifdef EXCLUDE_INSIGHT
        Logger::warning(
            "The Insight component has been excluded from this build. All Insight related configuration parameters "
            "will be ignored."
        );
#endif

        this->startup();

#ifndef EXCLUDE_INSIGHT
        if (this->insightConfig->activateOnStartup) {
            this->activateInsight();
        }
#endif
        this->checkBloomVersion();

        /*
         * We can't run our own event loop here - we have to use Qt's event loop. But we still need to be able to
         * process our events. To address this, we use a QTimer to dispatch our events on an interval.
         *
         * This allows us to use Qt's event loop whilst still being able to process our own events.
         */
        auto* eventDispatchTimer = new QTimer(&(this->qtApplication));
        QObject::connect(eventDispatchTimer, &QTimer::timeout, this, &Application::dispatchEvents);
        eventDispatchTimer->start(100);

        this->qtApplication.exec();

    } catch (const InvalidConfig& exception) {
        Logger::error("Invalid project configuration (bloom.yaml) - " + exception.getMessage());

    } catch (const Exception& exception) {
        Logger::error(exception.getMessage());
    }

    this->shutdown();
    return EXIT_SUCCESS;
}

std::map<std::string, std::function<int()>> Application::getCommandHandlersByCommandName() {
    return std::map<std::string, std::function<int()>> {
        {
            "--help",
            std::bind(&Application::presentHelpText, this)
        },
        {
            "-h",
            std::bind(&Application::presentHelpText, this)
        },
        {
            "--version",
            std::bind(&Application::presentVersionText, this)
        },
        {
            "-v",
            std::bind(&Application::presentVersionText, this)
        },
        {
            "--version-machine",
            std::bind(&Application::presentVersionMachineText, this)
        },
        {
            "init",
            std::bind(&Application::initProject, this)
        },
        {
            "--target-list-machine",
            std::bind(&Application::presentTargetListMachine, this)
        },
    };
}

void Application::startup() {
    auto& applicationEventListener = this->applicationEventListener;
    EventManager::registerListener(applicationEventListener);
    applicationEventListener->registerCallbackForEventType<Events::ShutdownApplication>(
        std::bind(&Application::onShutdownApplicationRequest, this, std::placeholders::_1)
    );

    this->loadProjectSettings();
    this->loadProjectConfiguration();
    Logger::configure(this->projectConfig.value());

    Logger::debug("Bloom version: " + Application::VERSION.toString());

    this->startSignalHandler();

    Logger::info("Selected environment: \"" + this->selectedEnvironmentName + "\"");
    Logger::debug("Number of environments extracted from config: "
        + std::to_string(this->projectConfig->environments.size()));

    applicationEventListener->registerCallbackForEventType<Events::TargetControllerThreadStateChanged>(
        std::bind(&Application::onTargetControllerThreadStateChanged, this, std::placeholders::_1)
    );

    applicationEventListener->registerCallbackForEventType<Events::DebugServerThreadStateChanged>(
        std::bind(&Application::onDebugServerThreadStateChanged, this, std::placeholders::_1)
    );

    applicationEventListener->registerCallbackForEventType<Events::DebugSessionFinished>(
        std::bind(&Application::onDebugSessionFinished, this, std::placeholders::_1)
    );

#ifndef EXCLUDE_INSIGHT
    applicationEventListener->registerCallbackForEventType<Events::InsightActivationRequested>(
        std::bind(&Application::onInsightActivationRequest, this, std::placeholders::_1)
    );

    applicationEventListener->registerCallbackForEventType<Events::InsightMainWindowClosed>(
        std::bind(&Application::onInsightMainWindowClosed, this, std::placeholders::_1)
    );
#endif
    this->startTargetController();
    this->startDebugServer();

    Thread::threadState = ThreadState::READY;
}

void Application::shutdown() {
    const auto appState = Thread::getThreadState();
    if (appState == ThreadState::STOPPED || appState == ThreadState::SHUTDOWN_INITIATED) {
        return;
    }

    Thread::threadState = ThreadState::SHUTDOWN_INITIATED;
    Logger::info("Shutting down Bloom");

    this->stopDebugServer();
    this->stopTargetController();
    this->stopSignalHandler();

    try {
        this->saveProjectSettings();

    } catch (const Exception& exception) {
        Logger::error("Failed to save project settings - " + exception.getMessage());
    }

    Thread::threadState = ThreadState::STOPPED;
}

void Application::triggerShutdown() {
#ifndef EXCLUDE_INSIGHT
    if (this->insight != nullptr) {
        this->insight->shutdown();
    }
#endif

    this->qtApplication.exit(EXIT_SUCCESS);
}

void Application::loadProjectSettings() {
    const auto projectSettingsPath = Services::PathService::projectSettingsPath();
    auto jsonSettingsFile = QFile(QString::fromStdString(projectSettingsPath));

    if (jsonSettingsFile.exists()) {
        try {
            if (!jsonSettingsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                throw Exception("Failed to open settings file.");
            }

            this->projectSettings = ProjectSettings(
                QJsonDocument::fromJson(jsonSettingsFile.readAll()).object()
            );
            jsonSettingsFile.close();

            return;

        } catch (const std::exception& exception) {
            Logger::error(
                "Failed to load project settings from " + projectSettingsPath + " - " + exception.what()
            );
        }
    }

    this->projectSettings = ProjectSettings();
}

void Application::saveProjectSettings() {
    if (!this->projectSettings.has_value()) {
        return;
    }

    const auto projectSettingsPath = Services::PathService::projectSettingsPath();
    auto jsonSettingsFile = QFile(QString::fromStdString(projectSettingsPath));

    Logger::debug("Saving project settings to " + projectSettingsPath);

    QDir().mkpath(QString::fromStdString(Services::PathService::projectSettingsDirPath()));

    try {
        const auto jsonDocument = QJsonDocument(this->projectSettings->toJson());

        if (!jsonSettingsFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
            throw Exception(
                "Failed to open/create settings file (" + projectSettingsPath + "). Check file permissions."
            );
        }

        jsonSettingsFile.write(jsonDocument.toJson());
        jsonSettingsFile.close();

    } catch (const Exception& exception) {
        Logger::error(
            "Failed to save project settings -  " + exception.getMessage()
        );
    }
}

void Application::loadProjectConfiguration() {
    auto configFile = QFile(QString::fromStdString(Services::PathService::projectConfigPath()));

    if (!configFile.exists()) {
        throw Exception(
            "Bloom configuration file (bloom.yaml) not found. Working directory: "
                + Services::PathService::projectDirPath()
        );
    }

    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw InvalidConfig(
            "Failed to open Bloom configuration file. Working directory: " + Services::PathService::projectDirPath()
        );
    }

    try {
        const auto configNode = YAML::Load(configFile.readAll().toStdString());
        configFile.close();

        this->projectConfig = ProjectConfig(configNode);

    } catch (const YAML::Exception& exception) {
        throw InvalidConfig(exception.msg);
    }

    // Validate the selected environment
    const auto selectedEnvironmentIt = this->projectConfig->environments.find(this->selectedEnvironmentName);
    if (selectedEnvironmentIt == this->projectConfig->environments.end()) {
        throw InvalidConfig(
            "Environment (\"" + this->selectedEnvironmentName + "\") not found in configuration."
        );
    }

    this->environmentConfig = selectedEnvironmentIt->second;

#ifndef EXCLUDE_INSIGHT
    this->insightConfig = this->environmentConfig->insightConfig.value_or(this->projectConfig->insightConfig);
#endif

    if (this->environmentConfig->debugServerConfig.has_value()) {
        this->debugServerConfig = this->environmentConfig->debugServerConfig.value();

    } else if (this->projectConfig->debugServerConfig.has_value()) {
        this->debugServerConfig = this->projectConfig->debugServerConfig.value();

    } else {
        throw InvalidConfig("Debug server configuration missing.");
    }
}

int Application::presentHelpText() {
    /*
     * Silence all logging here, as we're just to display the help text and then exit the application. Any
     * further logging will just be noise.
     */
    Logger::silence();

    // The file help.txt is included in Bloom's binary, as a resource. See the root-level CMakeLists.txt for more.
    auto helpFile = QFile(QString::fromStdString(Services::PathService::compiledResourcesPath() + "/resources/help.txt"));

    if (!helpFile.open(QIODevice::ReadOnly)) {
        // This should never happen - if it does, something has gone very wrong
        throw Exception(
            "Failed to open help file - please report this issue at " + Services::PathService::homeDomainName()
                + "/report-issue"
        );
    }

    std::cout << "Bloom v" << Application::VERSION.toString() << "\n";
    std::cout << QTextStream(&helpFile).readAll().toUtf8().constData() << "\n";
    return EXIT_SUCCESS;
}

int Application::presentVersionText() {
    Logger::silence();

    std::cout << "Bloom v" << Application::VERSION.toString() << "\n";

#ifdef EXCLUDE_INSIGHT
    std::cout << "Insight has been excluded from this build.\n";
#endif

#ifdef BLOOM_DEBUG_BUILD
    std::cout << "DEBUG BUILD - Compilation timestamp: " << __DATE__ << " " << __TIME__ << "\n";
#endif

    std::cout << Services::PathService::homeDomainName() + "/\n";
    std::cout << "Nav Mohammed\n";
    return EXIT_SUCCESS;
}

int Application::presentVersionMachineText() {
    Logger::silence();

    auto insightAvailable = true;
#ifdef EXCLUDE_INSIGHT
    insightAvailable = false;
#endif

    std::cout << QJsonDocument(QJsonObject({
        {"version", QString::fromStdString(Application::VERSION.toString())},
        {"components", QJsonObject({
            {"major", Application::VERSION.major},
            {"minor", Application::VERSION.minor},
            {"patch", Application::VERSION.patch},
        })},
        {"insightAvailable", insightAvailable},
    })).toJson().toStdString();

    return EXIT_SUCCESS;
}

int Application::presentTargetListMachine() {
    Logger::silence();

    using Targets::TargetFamily;
    static const auto targetFamilyNames = BiMap<TargetFamily, QString>({
        {TargetFamily::AVR_8, "AVR8"},
        {TargetFamily::RISC_V, "RISC-V"},
    });

    auto output = QJsonArray();

    for (const auto& [configValue, descriptor] : Targets::TargetDescription::TargetDescriptionFile::mapping()) {
        output.push_back(QJsonObject({
            {"name" , QString::fromStdString(descriptor.targetName)},
            {"family" , targetFamilyNames.at(descriptor.targetFamily)},
            {"configurationValue" , QString::fromStdString(configValue)},
        }));
    }

    std::cout << QJsonDocument(output).toJson().toStdString();

    return EXIT_SUCCESS;
}

int Application::initProject() {
    auto configFile = QFile(QString::fromStdString(Services::PathService::projectConfigPath()));

    if (configFile.exists()) {
        throw Exception("Bloom configuration file (bloom.yaml) already exists in working directory.");
    }

    /*
     * The file bloom.template.yaml is just a template Bloom config file that is included in Bloom's binary, as a
     * resource. See the root-level CMakeLists.txt for more.
     *
     * We simply copy the template file into the user's working directory.
     */
    auto templateConfigFile = QFile(
        QString::fromStdString(Services::PathService::compiledResourcesPath()+ "/resources/bloom.template.yaml")
    );

    if (!templateConfigFile.open(QIODevice::ReadOnly)) {
        throw Exception(
            "Failed to open template configuration file - please report this issue at "
                + Services::PathService::homeDomainName() + "/report-issue"
        );
    }

    if (!configFile.open(QIODevice::ReadWrite)) {
        throw Exception("Failed to create Bloom configuration file (bloom.yaml)");
    }

    configFile.write(templateConfigFile.readAll());

    configFile.close();
    templateConfigFile.close();

    Logger::info("Bloom configuration file (bloom.yaml) created in working directory.");
    return EXIT_SUCCESS;
}

void Application::startSignalHandler() {
    this->signalHandlerThread = std::thread(&SignalHandler::run, std::ref(this->signalHandler));
}

void Application::stopSignalHandler() {
    const auto shThreadState = this->signalHandler.getThreadState();

    if (shThreadState != ThreadState::STOPPED && shThreadState != ThreadState::UNINITIALISED) {
        this->signalHandler.triggerShutdown();

        /*
         * Send meaningless signal to the SignalHandler thread to have it shutdown. The signal will pull it out of
         * a blocking state and allow it to action the shutdown. See SignalHandler::run() for more.
         */
        pthread_kill(this->signalHandlerThread.native_handle(), SIGUSR1);
    }

    if (this->signalHandlerThread.joinable()) {
        Logger::debug("Joining SignalHandler thread");
        this->signalHandlerThread.join();
        Logger::debug("SignalHandler thread joined");
    }
}

void Application::startTargetController() {
    this->targetController = std::make_unique<TargetController::TargetControllerComponent>(
        this->projectConfig.value(),
        this->environmentConfig.value()
    );

    this->targetControllerThread = std::thread(
        &TargetController::TargetControllerComponent::run,
        this->targetController.get()
    );

    const auto tcStateChangeEvent = this->applicationEventListener->waitForEvent<
        Events::TargetControllerThreadStateChanged
    >();

    if (!tcStateChangeEvent.has_value() || tcStateChangeEvent->get()->getState() != ThreadState::READY) {
        throw Exception("TargetController failed to start up");
    }
}

void Application::stopTargetController() {
    if (this->targetController == nullptr) {
        return;
    }

    const auto tcThreadState = this->targetController->getThreadState();
    if (tcThreadState == ThreadState::STARTING || tcThreadState == ThreadState::READY) {
        EventManager::triggerEvent(std::make_shared<Events::ShutdownTargetController>());
        this->applicationEventListener->waitForEvent<Events::TargetControllerThreadStateChanged>(
            std::chrono::milliseconds(10000)
        );
    }

    if (this->targetControllerThread.joinable()) {
        Logger::debug("Joining TargetController thread");
        this->targetControllerThread.join();
        Logger::debug("TargetController thread joined");
    }
}

void Application::startDebugServer() {
    this->debugServer = std::make_unique<DebugServer::DebugServerComponent>(
        this->debugServerConfig.value()
    );

    this->debugServerThread = std::thread(
        &DebugServer::DebugServerComponent::run,
        this->debugServer.get()
    );

    const auto dsStateChangeEvent = this->applicationEventListener->waitForEvent<
        Events::DebugServerThreadStateChanged
    >();

    if (!dsStateChangeEvent.has_value() || dsStateChangeEvent->get()->getState() != ThreadState::READY) {
        throw Exception("DebugServer failed to start up");
    }
}

void Application::stopDebugServer() {
    if (this->debugServer == nullptr) {
        return;
    }

    const auto debugServerState = this->debugServer->getThreadState();
    if (debugServerState == ThreadState::STARTING || debugServerState == ThreadState::READY) {
        EventManager::triggerEvent(std::make_shared<Events::ShutdownDebugServer>());
        this->applicationEventListener->waitForEvent<Events::DebugServerThreadStateChanged>(
            std::chrono::milliseconds(5000)
        );
    }

    if (this->debugServerThread.joinable()) {
        Logger::debug("Joining DebugServer thread");
        this->debugServerThread.join();
        Logger::debug("DebugServer thread joined");
    }
}

void Application::dispatchEvents() {
    this->applicationEventListener->dispatchCurrentEvents();
}

void Application::checkBloomVersion() {
    const auto currentVersionNumber = Application::VERSION;

    auto* networkAccessManager = new QNetworkAccessManager(this);
    auto queryVersionEndpointUrl = QUrl(QString::fromStdString(Services::PathService::homeDomainName() + "/latest-version"));
    queryVersionEndpointUrl.setScheme("http");
    queryVersionEndpointUrl.setQuery(QUrlQuery({
        {"currentVersionNumber", QString::fromStdString(currentVersionNumber.toString())}
    }));

    QObject::connect(
        networkAccessManager,
        &QNetworkAccessManager::finished,
        this,
        [this, currentVersionNumber] (QNetworkReply* response) {
            const auto jsonResponseObject = QJsonDocument::fromJson(response->readAll()).object();
            const auto latestVersionNumber = VersionNumber(jsonResponseObject.value("latestVersionNumber").toString());

            if (latestVersionNumber > currentVersionNumber) {
                Logger::warning(
                    "Bloom v" + latestVersionNumber.toString()
                        + " is available to download - upgrade via " + Services::PathService::homeDomainName()
                );
            }
        }
    );

    networkAccessManager->get(QNetworkRequest(queryVersionEndpointUrl));
}

#ifndef EXCLUDE_INSIGHT
void Application::activateInsight() {
    assert(!this->insight);

    this->insight = std::make_unique<Insight>(
        *(this->applicationEventListener),
        this->projectConfig.value(),
        this->environmentConfig.value(),
        this->insightConfig.value(),
        this->projectSettings.value().insightSettings,
        &(this->qtApplication)
    );
}

void Application::onInsightActivationRequest(const Events::InsightActivationRequested&) {
    if (this->insight) {
        // Insight has already been activated
        this->insight->activateMainWindow();
        return;
    }

    this->activateInsight();
}

void Application::onInsightMainWindowClosed(const Events::InsightMainWindowClosed& event) {
    if (this->insightConfig->shutdownOnClose) {
        this->triggerShutdown();
    }
}
#endif

void Application::onShutdownApplicationRequest(const Events::ShutdownApplication&) {
    Logger::debug("ShutdownApplication event received.");
    this->triggerShutdown();
}

void Application::onTargetControllerThreadStateChanged(const Events::TargetControllerThreadStateChanged& event) {
    if (event.getState() == ThreadState::STOPPED || event.getState() == ThreadState::SHUTDOWN_INITIATED) {
        // TargetController has unexpectedly shutdown.
        this->triggerShutdown();
    }
}

void Application::onDebugServerThreadStateChanged(const Events::DebugServerThreadStateChanged& event) {
    if (event.getState() == ThreadState::STOPPED || event.getState() == ThreadState::SHUTDOWN_INITIATED) {
        // DebugServer has unexpectedly shutdown - it must have encountered a fatal error.
        this->triggerShutdown();
    }
}

void Application::onDebugSessionFinished(const Events::DebugSessionFinished& event) {
    if (this->environmentConfig->shutdownPostDebugSession || Services::ProcessService::isManagedByClion()) {
        this->triggerShutdown();
    }
}
