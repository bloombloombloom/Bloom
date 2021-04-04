#pragma once

#include <memory>
#include <map>
#include <string>
#include <functional>
#include <QtCore/QtCore>
#include <thread>

#include "src/SignalHandler/SignalHandler.hpp"
#include "src/TargetController/TargetController.hpp"
#include "src/DebugServers/GdbRsp/AvrGdbRsp/AvrGdbRsp.hpp"
#include "src/Insight/Insight.hpp"
#include "src/Helpers/Thread.hpp"
#include "src/Logger/Logger.hpp"
#include "src/ApplicationConfig.hpp"
#include "src/Exceptions/Exception.hpp"
#include "src/EventManager/EventListener.hpp"
#include "src/EventManager/Events/Events.hpp"

namespace Bloom
{
    using namespace DebugServers;

    class Application: public Thread
    {
    public:
        static const inline std::string VERSION_STR = "0.0.1";

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

        SignalHandler signalHandler = SignalHandler(this->eventManager);
        std::thread signalHandlerThread;

        TargetController targetController = TargetController(this->eventManager);
        std::thread targetControllerThread;

        std::unique_ptr<DebugServer> debugServer = nullptr;
        std::thread debugServerThread;

        Insight insight = Insight(this->eventManager);

        ApplicationConfig applicationConfig;
        EnvironmentConfig environmentConfig;
        DebugServerConfig debugServerConfig;
        InsightConfig insightConfig;

        std::optional<std::string> selectedEnvironmentName;

        auto getCommandToCallbackMapping() {
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
         * Extracts config from the user's JSON config file and generates an ApplicationConfig object.
         *
         * @see ApplicationConfig declaration for more on this.
         * @return
         */
        static ApplicationConfig extractConfig();

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
         * Prepares a dedicated thread for the TargetController and kicks it off.
         */
        void startTargetController();

        /**
         * Invokes a clean shutdown of the TargetController. The TargetController should disconnect from the target
         * and debug tool in a clean and safe manner, ensuring that both are left in a sensible state.
         *
         * This will join the TargetController thread.
         */
        void stopTargetController();

        void startDebugServer();

        void stopDebugServer();

    public:
        explicit Application() = default;

        auto getSupportedDebugServers() {
            return std::map<std::string, std::function<std::unique_ptr<DebugServer>()>> {
                {
                    "avr-gdb-rsp",
                    [this]() -> std::unique_ptr<DebugServer> {
                        return std::make_unique<DebugServers::Gdb::AvrGdbRsp>(this->eventManager);
                    }
                },
            };
        };

        int run(const std::vector<std::string>& arguments);

        void handleTargetControllerStateChangedEvent(EventPointer<Events::TargetControllerStateChanged> event);
        void onDebugServerStateChanged(EventPointer<Events::DebugServerStateChanged> event);
        void handleShutdownApplicationEvent(EventPointer<Events::ShutdownApplication>);

        /**
         * Returns the path to the directory in which the Bloom binary resides.
         *
         * @return
         */
        static std::string getApplicationDirPath();

        /**
         * Returns the path to the Resources directory, located in the application directory.
         *
         * @return
         */
        static std::string getResourcesDirPath();

        /**
         * Checks if the current effective user running Bloom has root privileges.
         *
         * @return
         */
        static bool isRunningAsRoot();
    };
}
