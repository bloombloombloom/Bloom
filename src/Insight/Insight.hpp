#pragma once

#include <QtCore>
#include <QApplication>
#include "UserInterfaces/InsightWindow/InsightWindow.hpp"

#include "src/Helpers/Thread.hpp"
#include "src/ApplicationConfig.hpp"
#include "src/EventManager/EventManager.hpp"
#include "src/EventManager/EventListener.hpp"
#include "src/TargetController/TargetControllerConsole.hpp"

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
    private:
        std::string qtApplicationName = "Bloom";
        std::array<char*, 1> qtApplicationArgv = {this->qtApplicationName.data()};
        int qtApplicationArgc = 1;

        ApplicationConfig applicationConfig;
        EnvironmentConfig environmentConfig;
        InsightConfig insightConfig;

        EventManager& eventManager;
        EventListenerPointer eventListener = std::make_shared<EventListener>("InsightEventListener");

        QApplication application =  QApplication(this->qtApplicationArgc, this->qtApplicationArgv.data());
        InsightWindow mainWindow;

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

    public:
        explicit Insight(EventManager& eventManager): eventManager(eventManager) {};

        void setApplicationConfig(const ApplicationConfig& applicationConfig) {
            this->applicationConfig = applicationConfig;
        }

        void setEnvironmentConfig(const EnvironmentConfig& environmentConfig) {
            this->environmentConfig = environmentConfig;
        }

        void setInsightConfig(const InsightConfig& insightConfig) {
            this->insightConfig = insightConfig;
        }

        /**
         * Entry point for Insight.
         */
        void run();

        /**
         * Shuts down Insight. Called when the user closes the Insight window.
         */
        void shutdown();

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
