#pragma once

#include <memory>
#include <map>
#include <string>
#include <optional>
#include <functional>
#include <QtCore/QtCore>
#include <QCoreApplication>
#ifndef EXCLUDE_INSIGHT
#include <QApplication>
#endif
#include <thread>

#include "src/Helpers/Thread.hpp"

#include "src/TargetController/TargetControllerComponent.hpp"
#include "src/DebugServer/DebugServerComponent.hpp"

#ifndef EXCLUDE_INSIGHT
#include "src/Insight/Insight.hpp"
#endif

#include "src/SignalHandler/SignalHandler.hpp"

#include "src/ProjectConfig.hpp"
#include "src/ProjectSettings.hpp"

#include "src/EventManager/EventListener.hpp"
#include "src/EventManager/Events/Events.hpp"

#include "src/VersionNumber.hpp"

class Application: public QObject, public Thread
{
    Q_OBJECT

public:
    static const inline VersionNumber VERSION = VersionNumber{std::string{BLOOM_VERSION}};

    explicit Application(std::vector<std::string>&& arguments);
    int run();

private:
    std::vector<std::string> arguments;

    std::string qtApplicationName = "Bloom";
    std::array<char*, 1> qtApplicationArgv = {this->qtApplicationName.data()};
    int qtApplicationArgc = 1;
#ifndef EXCLUDE_INSIGHT
    std::unique_ptr<QApplication> qtApplication = nullptr;
#else
    std::unique_ptr<QCoreApplication> qtApplication = nullptr;
#endif

    EventListenerPointer applicationEventListener = std::make_shared<EventListener>("ApplicationEventListener");

    /**
     * The SignalHandler deals with any UNIX signals. It runs on a dedicated thread. All other threads
     * ignore UNIX signals.
     *
     * See the SignalHandler class for more on this.
     */
    SignalHandler signalHandler = {};
    std::thread signalHandlerThread;

    /**
     * The TargetController possesses full control of the connected debug tool and target. It runs on a
     * dedicated thread.
     *
     * See the TargetController class for more on this.
     *
     * I could have used std::optional here, for the late initialisation, but given that we're using
     * std::unique_ptr for the debug server (for polymorphism), I thought I'd keep it consistent.
     */
    std::unique_ptr<TargetController::TargetControllerComponent> targetController = nullptr;
    std::thread targetControllerThread;

    /**
     * The DebugServer exposes an interface to the connected target, to third-party software such as IDEs. It runs
     * on a dedicated thread.
     *
     * See the DebugServer and GdbRspDebugServer class for more on this.
     */
    std::unique_ptr<DebugServer::DebugServerComponent> debugServer = nullptr;
    std::thread debugServerThread;

#ifndef EXCLUDE_INSIGHT
    /**
     * Insight is a small Qt application that serves a GUI to the user. It occupies the main thread, as well as a
     * single worker thread, and possibly other threads created by Qt.
     *
     * Insight is optional - users can disable it via their project configuration.
     *
     * When the user closes the Insight GUI, control of the main thread is returned to Application::run(). See the
     * Insight class for more on this.
     *
     * Because Insight is optional, we only construct the Insight object when we need it. We can't use
     * std::optional here because the Insight class extends QObject, which disables the copy constructor and
     * the assignment operator. So we use an std::unique_ptr instead, which is perfectly fine for this use case,
     * as we want to manage the lifetime of the object here.
     */
    std::unique_ptr<Insight> insight = nullptr;

    /**
     * Activation of the Insight GUI can be requested by triggering an InsightActivationRequested event.
     *
     * This flag will be set upon that event being triggered. The Insight GUI will be started shortly after.
     */
    bool insightActivationPending = false;
#endif

    /**
     * Configuration extracted from the user's project configuration file.
     *
     * See ProjectConfig.hpp for more on this.
     */
    std::optional<ProjectConfig> projectConfig;
    std::optional<EnvironmentConfig> environmentConfig;
    std::optional<DebugServerConfig> debugServerConfig;
    std::optional<InsightConfig> insightConfig;

    /**
     * Settings extracted from the settings file in the user's project root.
     */
    std::optional<ProjectSettings> projectSettings;

    /**
     * The project environment selected by the user.
     *
     * If an environment name is not provided as an argument when running Bloom, Bloom will fallback to the
     * environment named "default".
     */
    std::string selectedEnvironmentName = "default";

    /**
     * Some CLI arguments are interpreted as commands and thus require specific handler methods to be called.
     * This mapping maps command names to the appropriate handler methods. The mapped handler method is invoked
     * when the command name is provided as an argument from the CLI.
     *
     * See Application::run() for more on this.
     *
     * @return
     */
    std::map<std::string, std::function<int()>> getCommandHandlersByCommandName();

    /**
     * Kicks off the application.
     *
     * Will start the TargetController and DebugServer. And register the main application's event handlers.
     */
    void startup();

    /**
     * Will cleanly shutdown the application.
     */
    void shutdown();

    /**
     * Will trigger a clean shutdown.
     */
    void triggerShutdown();

    /**
     * Extracts or generates project settings.
     */
    void loadProjectSettings();

    /**
     * Saves the current project settings.
     */
    void saveProjectSettings();

    /**
     * Extracts the project config from the user's config file and populates the following members:
     *  - this->projectConfig
     *  - this->environmentConfig
     *  - this->debugServerConfig
     *  - this->insightConfig
     *
     * @see ProjectConfig declaration for more on this.
     */
    void loadProjectConfiguration();

    /**
     * Presents application help text to user.
     *
     * @return
     */
    int presentHelpText();

    /**
     * Presents the current Bloom version number to user.
     *
     * @return
     */
    int presentVersionText();

    /**
     * Presents the current Bloom version number, in JSON format.
     *
     * @return
     */
    int presentVersionMachineText();

    /**
     * Presents Bloom's capabilities, in JSON format.
     *
     * @return
     */
    int presentCapabilitiesMachine();

    /**
     * Initialises a project in the user's working directory.
     *
     * @return
     */
    int initProject();

    /**
     * Prepares a dedicated thread for the SignalHandler and kicks it off with a call to SignalHandler::run().
     */
    void startSignalHandler();

    /**
     * Sends a shutdown request to the SignalHandler and waits on the dedicated thread to exit.
     */
    void stopSignalHandler();

    /**
     * Prepares a dedicated thread for the TargetController and kicks it off with a call to
     * TargetControllerComponent::run().
     */
    void startTargetController();

    /**
     * Invokes a clean shutdown of the TargetController. The TargetController should disconnect from the target
     * and debug tool in a clean and safe manner, ensuring that both are left in a sensible state.
     *
     * This will join the TargetController thread.
     */
    void stopTargetController();

    /**
     * Prepares a dedicated thread for the DebugServer and kicks it off with a call to DebugServer::run().
     */
    void startDebugServer();

    /**
     * Sends a shutdown request to the DebugServer thread and waits on it to exit.
     */
    void stopDebugServer();

    /**
     * Dispatches any pending events. This function will be called periodically, via a QTimer.
     */
    void dispatchEvents();

    /**
     * Queries the Bloom server for the latest version number. If the current version number doesn't match the
     * latest version number returned by the server, we'll display a warning in the logs to instruct the user to
     * upgrade.
     */
    void checkBloomVersion();

#ifndef EXCLUDE_INSIGHT
    /**
     * Activate the Insight GUI.
     *
     * This function should never be called more than once.
     */
    void activateInsight();

    /**
     * Handles requests to start the Insight GUI.
     */
    void onInsightActivationRequest(const Events::InsightActivationRequested&);

    /**
     * Performs any post-insight-closed actions.
     *
     * @param event
     */
    void onInsightMainWindowClosed(const Events::InsightMainWindowClosed& event);
#endif

    /**
     * Triggers a shutdown of Bloom and all of its components.
     */
    void onShutdownApplicationRequest(const Events::ShutdownApplication&);

    /**
     * If the TargetController unexpectedly shuts down, the rest of the application will follow.
     *
     * @param event
     */
    void onTargetControllerThreadStateChanged(const Events::TargetControllerThreadStateChanged& event);

    /**
     * Same goes for the DebugServer - it should never shutdown unless a shutdown request was issued. If it does,
     * something horrible has happened and so we shutdown the rest of the application in response.
     *
     * @param event
     */
    void onDebugServerThreadStateChanged(const Events::DebugServerThreadStateChanged& event);

    /**
     * If configured to do so, Bloom will shutdown upon the end of the current debug session.
     *
     * @param event
     */
    void onDebugSessionFinished(const Events::DebugSessionFinished& event);
};
