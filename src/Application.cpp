#include "Application.hpp"

#include <iostream>
#include <csignal>
#include <thread>
#include <QJsonDocument>
#include <unistd.h>
#include <filesystem>

#include "src/Logger/Logger.hpp"
#include "src/Helpers/Paths.hpp"
#include "src/Exceptions/InvalidConfig.hpp"

namespace Bloom
{
    using namespace Exceptions;

    int Application::run(const std::vector<std::string>& arguments) {
        try {
            this->setName("Bloom");

            if (!arguments.empty()) {
                const auto& firstArg = arguments.front();
                const auto commandsToCallbackMapping = this->getCommandToHandlerMapping();

                if (commandsToCallbackMapping.contains(firstArg)) {
                    // User has passed an argument that maps to a command callback - invoke the callback and shutdown
                    auto returnValue = commandsToCallbackMapping.at(firstArg)();

                    this->shutdown();
                    return returnValue;
                }

                // If the first argument didn't map to a command, we assume it's an environment name
                this->selectedEnvironmentName = firstArg;
            }

#ifdef BLOOM_DEBUG_BUILD
            Logger::warning("This is a debug build - some functions may not work as expected.");
#endif

            this->startup();

            if (this->insightConfig->insightEnabled) {
                this->insight = std::make_unique<Insight>(
                    *(this->applicationEventListener),
                    this->projectConfig.value(),
                    this->environmentConfig.value(),
                    this->insightConfig.value(),
                    this->projectSettings.value().insightSettings
                );

                /*
                 * Before letting Insight occupy the main thread, process any pending events that accumulated
                 * during startup.
                 */
                this->applicationEventListener->dispatchCurrentEvents();

                if (Thread::getThreadState() == ThreadState::READY) {
                    this->insight->run();
                    Logger::debug("Insight closed");
                }

                this->shutdown();
                return EXIT_SUCCESS;
            }

            // Main event loop
            while (Thread::getThreadState() == ThreadState::READY) {
                this->applicationEventListener->waitAndDispatch();
            }

        } catch (const InvalidConfig& exception) {
            Logger::error(exception.getMessage());

        } catch (const Exception& exception) {
            Logger::error(exception.getMessage());
        }

        this->shutdown();
        return EXIT_SUCCESS;
    }

    bool Application::isRunningAsRoot() {
        return geteuid() == 0;
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

        this->blockAllSignalsOnCurrentThread();
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

        this->startTargetController();
        this->startDebugServer();

        Thread::setThreadState(ThreadState::READY);
    }

    void Application::shutdown() {
        auto appState = Thread::getThreadState();
        if (appState == ThreadState::STOPPED || appState == ThreadState::SHUTDOWN_INITIATED) {
            return;
        }

        Thread::setThreadState(ThreadState::SHUTDOWN_INITIATED);
        Logger::info("Shutting down Bloom");

        if (this->insight != nullptr) {
            this->insight->shutdown();
        }

        this->stopDebugServer();
        this->stopTargetController();
        this->stopSignalHandler();

        this->saveProjectSettings();
        Thread::setThreadState(ThreadState::STOPPED);
    }

    void Application::loadProjectSettings() {
        const auto projectSettingsPath = Paths::projectSettingsPath();
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

        const auto projectSettingsPath = Paths::projectSettingsPath();
        auto jsonSettingsFile = QFile(QString::fromStdString(projectSettingsPath));

        Logger::debug("Saving project settings to " + projectSettingsPath);

        QDir().mkpath(QString::fromStdString(Paths::projectSettingsDirPath()));

        try {
            const auto jsonDocument = QJsonDocument(this->projectSettings->toJson());

            if (!jsonSettingsFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
                throw Exception(
                    "failed to open/create settings file (" + projectSettingsPath + "). Check file permissions."
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
        auto jsonConfigFile = QFile(QString::fromStdString(Paths::projectConfigPath()));

        if (!jsonConfigFile.exists()) {
            throw InvalidConfig("Bloom configuration file (bloom.json) not found. Working directory: "
                + Paths::projectDirPath());
        }

        if (!jsonConfigFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            throw InvalidConfig("Failed to load Bloom configuration file (bloom.json) Working directory: "
                + Paths::projectDirPath());
        }

        auto jsonObject = QJsonDocument::fromJson(jsonConfigFile.readAll()).object();
        jsonConfigFile.close();

        this->projectConfig = ProjectConfig(jsonObject);

        // Validate the selected environment
        if (!this->projectConfig->environments.contains(this->selectedEnvironmentName)) {
            throw InvalidConfig(
                "Environment (\"" + this->selectedEnvironmentName + "\") not found in configuration."
            );
        }

        this->environmentConfig = this->projectConfig->environments.at(this->selectedEnvironmentName);

        if (this->environmentConfig->insightConfig.has_value()) {
            this->insightConfig = this->environmentConfig->insightConfig.value();

        } else if (this->projectConfig->insightConfig.has_value()) {
            this->insightConfig = this->projectConfig->insightConfig.value();

        } else {
            throw InvalidConfig("Insight configuration missing.");
        }

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

        // The file help.txt is included in the binary image as a resource. See src/resource.qrc
        auto helpFile = QFile(QString::fromStdString(Paths::compiledResourcesPath() + "/resources/help.txt"));

        if (!helpFile.open(QIODevice::ReadOnly)) {
            // This should never happen - if it does, something has gone very wrong
            throw Exception(
                "Failed to open help file - please report this issue at " + Paths::homeDomainName()
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

#ifdef BLOOM_DEBUG_BUILD
        std::cout << "DEBUG BUILD - Compilation timestamp: " << __DATE__ << " " << __TIME__ << "\n";
#endif

        std::cout << Paths::homeDomainName() + "/\n";
        std::cout << "Nav Mohammed\n";
        return EXIT_SUCCESS;
    }

    int Application::presentVersionMachineText() {
        Logger::silence();

        std::cout << QJsonDocument(QJsonObject({
            {"version", QString::fromStdString(Application::VERSION.toString())},
            {"components", QJsonObject({
                {"major", Application::VERSION.getMajor()},
                {"minor", Application::VERSION.getMinor()},
                {"patch", Application::VERSION.getPatch()},
            })},
        })).toJson().toStdString();

        return EXIT_SUCCESS;
    }

    int Application::initProject() {
        auto configFile = QFile(
            QString::fromStdString(std::filesystem::current_path().string() + "/bloom.json")
        );

        if (configFile.exists()) {
            throw Exception("Bloom configuration file (bloom.json) already exists in working directory.");
        }

        /*
         * The file bloom.template.json is just a template Bloom config file that is included in the binary image as
         * a resource. See src/resource.qrc
         *
         * We simply copy the template file into the user's working directory.
         */
        auto templateConfigFile = QFile(
            QString::fromStdString(Paths::compiledResourcesPath()+ "/resources/bloom.template.json")
        );

        if (!templateConfigFile.open(QIODevice::ReadOnly)) {
            throw Exception(
                "Failed to open template configuration file - please report this issue at "
                    + Paths::homeDomainName() + "/report-issue"
            );
        }

        if (!configFile.open(QIODevice::ReadWrite)) {
            throw Exception("Failed to create Bloom configuration file (bloom.json)");
        }

        configFile.write(templateConfigFile.readAll());

        configFile.close();
        templateConfigFile.close();

        Logger::info("Bloom configuration file (bloom.json) created in working directory.");
        return EXIT_SUCCESS;
    }

    void Application::startSignalHandler() {
        this->signalHandlerThread = std::thread(&SignalHandler::run, std::ref(this->signalHandler));
    }

    void Application::stopSignalHandler() {
        if (this->signalHandler.getThreadState() != ThreadState::STOPPED
            && this->signalHandler.getThreadState() != ThreadState::UNINITIALISED
        ) {
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

        auto tcStateChangeEvent =
            this->applicationEventListener->waitForEvent<Events::TargetControllerThreadStateChanged>();

        if (!tcStateChangeEvent.has_value() || tcStateChangeEvent->get()->getState() != ThreadState::READY) {
            throw Exception("TargetController failed to startup.");
        }
    }

    void Application::stopTargetController() {
        if (this->targetController == nullptr) {
            return;
        }

        auto targetControllerState = this->targetController->getThreadState();
        if (targetControllerState == ThreadState::STARTING || targetControllerState == ThreadState::READY) {
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

        auto dsStateChangeEvent = this->applicationEventListener->waitForEvent<Events::DebugServerThreadStateChanged>();

        if (!dsStateChangeEvent.has_value() || dsStateChangeEvent->get()->getState() != ThreadState::READY) {
            throw Exception("DebugServer failed to startup.");
        }
    }

    void Application::stopDebugServer() {
        if (this->debugServer == nullptr) {
            return;
        }

        auto debugServerState = this->debugServer->getThreadState();
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

    void Application::onShutdownApplicationRequest(const Events::ShutdownApplication&) {
        Logger::debug("ShutdownApplication event received.");
        this->shutdown();
    }

    void Application::onTargetControllerThreadStateChanged(const Events::TargetControllerThreadStateChanged& event) {
        if (event.getState() == ThreadState::STOPPED || event.getState() == ThreadState::SHUTDOWN_INITIATED) {
            // TargetController has unexpectedly shutdown - it must have encountered a fatal error.
            this->shutdown();
        }
    }

    void Application::onDebugServerThreadStateChanged(const Events::DebugServerThreadStateChanged& event) {
        if (event.getState() == ThreadState::STOPPED || event.getState() == ThreadState::SHUTDOWN_INITIATED) {
            // DebugServer has unexpectedly shutdown - it must have encountered a fatal error.
            this->shutdown();
        }
    }

    void Application::onDebugSessionFinished(const Events::DebugSessionFinished& event) {
        if (this->environmentConfig->shutdownPostDebugSession) {
            this->shutdown();
        }
    }
}
