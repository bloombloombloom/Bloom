#pragma once

#include <memory>
#include <map>
#include <string>
#include <functional>
#include <QtCore/QtCore>
#include <thread>

#include "src/Helpers/Thread.hpp"

#include "src/TargetController/TargetController.hpp"
#include "src/DebugServers/GdbRsp/AvrGdbRsp/AvrGdbRsp.hpp"
#include "src/Insight/Insight.hpp"
#include "src/SignalHandler/SignalHandler.hpp"

#include "src/ProjectConfig.hpp"
#include "src/ProjectSettings.hpp"

#include "src/EventManager/EventListener.hpp"
#include "src/EventManager/Events/Events.hpp"

#include "src/VersionNumber.hpp"

namespace Bloom
{
    /**
     * Bloom - a debug interface for embedded systems development on Linux.
     *
     * This is the main entry-point of execution for the Bloom program. The methods within will run on the main
     * thread. If Insight is enabled, execution will be passed over to Insight::run() upon startup.
     */
    class Application: public Thread
    {
    public:
        static const inline VersionNumber VERSION = VersionNumber(0, 5, 0);

        explicit Application() = default;

        /**
         * This mapping is used to map debug server names from project configuration files to polymorphic instances of
         * the DebugServer class.
         *
         * See Application::startDebugServer() for more on this.
         *
         * @return
         */
        auto getSupportedDebugServers() {
            return std::map<std::string, std::function<std::unique_ptr<DebugServers::DebugServer>()>> {
                {
                    "avr-gdb-rsp",
                    [this] () -> std::unique_ptr<DebugServers::DebugServer> {
                        return std::make_unique<DebugServers::Gdb::AvrGdbRsp>(
                            this->eventManager,
                            this->projectConfig.value(),
                            this->environmentConfig.value(),
                            this->debugServerConfig.value()
                        );
                    }
                },
            };
        };

        /**
         * Main entry-point for the Bloom program.
         *
         * @param arguments
         *  A vector of string arguments passed from the user via the cli.
         *
         * @return
         */
        int run(const std::vector<std::string>& arguments);

        /**
         * Checks if the current effective user running Bloom has root privileges.
         *
         * @return
         */
        static bool isRunningAsRoot();

    private:
        /**
         * This is the application wide event manager. It should be the only instance of the EventManager class at
         * any given time.
         *
         * It should be injected (by reference) into any other instances of classes that require use
         * of the EventManager.
         */
        EventManager eventManager = EventManager();
        EventListenerPointer applicationEventListener = std::make_shared<EventListener>("ApplicationEventListener");

        /**
         * The SignalHandler deals with any UNIX signals. It runs on a dedicated thread. All other threads
         * ignore UNIX signals.
         *
         * See the SignalHandler class for more on this.
         */
        SignalHandler signalHandler = SignalHandler(this->eventManager);
        std::thread signalHandlerThread;

        /**
         * The TargetController possesses full control of the connect debug tool and target. It runs on a
         * dedicated thread.
         *
         * See the TargetController class for more on this.
         *
         * I could have used std::optional here, for the late initialisation, but given that we're using
         * std::unique_ptr for the debug server (for polymorphism), I thought I'd keep it consistent and just use
         * std::unique_ptr for lazy loading.
         */
        std::unique_ptr<TargetController> targetController = nullptr;
        std::thread targetControllerThread;

        /**
         * The DebugServer exposes an interface to the connected target, to third-party software such as IDEs. It runs
         * on a dedicated thread.
         *
         * See the DebugServer and GdbRspDebugServer class for more on this.
         */
        std::unique_ptr<DebugServers::DebugServer> debugServer = nullptr;
        std::thread debugServerThread;

        /**
         * Insight is, effectively, a small Qt application that serves a GUI to the user. It occupies the main thread,
         * as well as a single worker thread, and possibly other threads created by Qt.
         *
         * Insight is optional - users can disable it via their project configuration.
         *
         * When the user closes the Insight GUI, control of the main thread is returned to Application::run(). How we
         * deal with the GUI being closed at this point depends on user configuration.
         *
         * See the Insight class for more on this.
         *
         * Because Insight is optional, we only construct the Insight object when we need it. We can't use
         * std::optional here because the Insight class extends QObject, which disables the copy constructor and
         * the assignment operator. So we use an std::unique_ptr instead, which is perfectly fine for this use case,
         * as we want to manage the lifetime of the object here.
         */
        std::unique_ptr<Insight> insight = nullptr;

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
         * This mapping maps command strings to the appropriate handler methods. The mapped handler method is invoked
         * when the command is provided as an argument from the CLI.
         *
         * See Application::run() for more on this.
         *
         * @return
         */
        auto getCommandToHandlerMapping() {
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
                    "init",
                    std::bind(&Application::initProject, this)
                },
            };
        }

        /**
         * Kicks off the application.
         *
         * Will start the TargetController and DebugServer. And register the main application's event handlers.
         */
        void startup();

        /**
         * Will cleanly shutdown the application. This should never fail.
         */
        void shutdown();

        /**
         * Extracts or generates project settings.
         */
        void loadProjectSettings();

        /**
         * Saves the current project settings.
         */
        void saveProjectSettings();

        /**
         * Extracts the project config from the user's JSON config file and populates the following members:
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
         * Prepares a dedicated thread for the TargetController and kicks it off with a call to TargetController::run().
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
         * Triggers a shutdown of Bloom and all of its components.
         */
        void onShutdownApplicationRequest(const Events::ShutdownApplication&);
    };
}
