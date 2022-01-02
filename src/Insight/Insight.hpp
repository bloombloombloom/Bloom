#pragma once

#include <QtCore>
#include <QApplication>

#include "src/Helpers/Thread.hpp"
#include "src/TargetController/TargetControllerConsole.hpp"
#include "src/Helpers/Paths.hpp"
#include "src/ProjectConfig.hpp"
#include "src/ProjectSettings.hpp"

#include "src/EventManager/EventManager.hpp"
#include "src/EventManager/EventListener.hpp"

#include "InsightWorker/InsightWorker.hpp"
#include "UserInterfaces/InsightWindow/InsightWindow.hpp"

namespace Bloom
{
    /**
     * The Insight component provides a GUI for insight into the target's GPIO state.
     * Insight relies heavily on the Qt framework - it's practically a small Qt application. Each supported target
     * package variant implements a custom Qt widget that presents the user with the current state of the target's GPIO
     * pins, as well as the ability to manipulate the pin states of output pins by clicking on them.
     *
     * The Insight component occupies the Bloom's main thread. See Application::run() for more.
     */
    class Insight: public QObject, public Thread
    {
        Q_OBJECT

    public:
        /**
         * Insight constructor.
         *
         * Note: We use the comma operator in the application() initializer to set the Qt::AA_ShareOpenGLContexts
         * attribute, as this is required by Qt before creating a QCoreApplication instance.
         *
         * @param eventManager
         */
        explicit Insight(
            EventManager& eventManager,
            const ProjectConfig& projectConfig,
            const EnvironmentConfig& environmentConfig,
            const InsightConfig& insightConfig,
            const InsightProjectSettings& insightProjectSettings
        ):
        eventManager(eventManager),
        projectConfig(projectConfig),
        environmentConfig(environmentConfig),
        insightConfig(insightConfig),
        insightProjectSettings(insightProjectSettings),
        application(
            (
                QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, false),
#ifndef BLOOM_DEBUG_BUILD
                QCoreApplication::addLibraryPath(QString::fromStdString(Paths::applicationDirPath() + "/plugins")),
#endif
                QApplication(this->qtApplicationArgc, this->qtApplicationArgv.data())
            )
        ) {};

        /**
         * Entry point for Insight.
         */
        void run();

    private:
        std::string qtApplicationName = "Bloom";
        std::array<char*, 1> qtApplicationArgv = {this->qtApplicationName.data()};
        int qtApplicationArgc = 1;

        ProjectConfig projectConfig;
        EnvironmentConfig environmentConfig;
        InsightConfig insightConfig;

        InsightProjectSettings insightProjectSettings;

        EventManager& eventManager;
        EventListenerPointer eventListener = std::make_shared<EventListener>("InsightEventListener");

        QApplication application;
        InsightWorker* insightWorker = new InsightWorker(this->eventManager);
        InsightWindow* mainWindow = new InsightWindow(
            *(this->insightWorker),
            this->environmentConfig,
            this->insightConfig
        );

        TargetControllerConsole targetControllerConsole = TargetControllerConsole(
            this->eventManager,
            *(this->eventListener)
        );

        /**
         * Insight consists of two threads - the main thread where the main Qt event loop runs (for the GUI), and
         * a single worker thread to handle any blocking/time-expensive operations.
         */
        QThread* workerThread = nullptr;

        void startup();

        /**
         * Shuts down Insight. Called when the user closes the Insight window.
         */
        void shutdown();

        /**
         * Queries the Bloom server for the latest version number. If the current version number doesn't match the
         * latest version number returned by the server, we'll display a warning in the logs to instruct the user to
         * upgrade.
         */
        void checkBloomVersion();

        /**
         * Because Insight occupies the main thread, it must handle any application shutdown requests.
         *
         * @param event
         */
        void onShutdownApplicationEvent(const Events::ShutdownApplication& event);

        /**
         * If the something horrible was to happen and the TC dies unexpectedly, Insight will shutdown in response.
         *
         * @param event
         */
        void onTargetControllerThreadStateChangedEvent(const Events::TargetControllerThreadStateChanged& event);

        /**
         * If something horrible was to happen and the DebugServer dies unexpectedly, Insight will shutdown in
         * response.
         *
         * @param event
         */
        void onDebugServerThreadStateChangedEvent(const Events::DebugServerThreadStateChanged& event);

        /**
         * Dispatches any events currently in the queue.
         *
         * Because Insight is effectively a Qt application, we cannot use our own event loop. We must use Qt's event
         * loop. We do this by utilizing a QTimer instance to call this method on an interval.
         * See Insight::startup() for more.
         */
        void dispatchEvents() {
            this->eventListener->dispatchCurrentEvents();
        };
    };
}
