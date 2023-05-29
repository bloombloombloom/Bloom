#pragma once

#include <QtCore>
#include <QApplication>
#include <cstdint>
#include <map>
#include <utility>
#include <QThread>
#include <QTimer>

#include "src/Helpers/Thread.hpp"
#include "src/Services/PathService.hpp"
#include "src/ProjectConfig.hpp"
#include "src/ProjectSettings.hpp"

#include "src/EventManager/EventManager.hpp"
#include "src/EventManager/EventListener.hpp"
#include "src/EventManager/Events/Events.hpp"

#include "src/Services/TargetControllerService.hpp"
#include "src/Targets/TargetState.hpp"

#include "InsightSignals.hpp"

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
    class Insight: public QObject
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
            EventListener& eventListener,
            const ProjectConfig& projectConfig,
            const EnvironmentConfig& environmentConfig,
            const InsightConfig& insightConfig,
            InsightProjectSettings& insightProjectSettings,
            QApplication* parent
        );

        /**
         * Entry point for Insight.
         */
        void activate();

        /**
         * Opens main window and obtains focus.
         */
        void showMainWindow();

        /**
         * Shuts down Insight. Called when the user closes the Insight window or a ShutdownApplication event is fired.
         */
        void shutdown();

    private:
        static constexpr std::uint8_t INSIGHT_WORKER_COUNT = 3;

        ProjectConfig projectConfig;
        EnvironmentConfig environmentConfig;
        InsightConfig insightConfig;

        InsightProjectSettings& insightProjectSettings;

        EventListener& eventListener;
        Services::TargetControllerService targetControllerService = Services::TargetControllerService();

        std::map<decltype(InsightWorker::id), std::pair<InsightWorker*, QThread*>> insightWorkersById;
        InsightWindow* mainWindow = new InsightWindow(
            this->environmentConfig,
            this->insightConfig,
            this->insightProjectSettings,
            this->targetControllerService.getTargetDescriptor()
        );

        Targets::TargetState lastTargetState = Targets::TargetState::UNKNOWN;
        bool targetStepping = false;
        QTimer* targetResumeTimer = nullptr;

        InsightSignals* insightSignals = InsightSignals::instance();

        /**
         * Queries the Bloom server for the latest version number. If the current version number doesn't match the
         * latest version number returned by the server, we'll display a warning in the logs to instruct the user to
         * upgrade.
         */
        void checkBloomVersion();

        void onInsightWindowActivated();
        void onTargetStoppedEvent(const Events::TargetExecutionStopped& event);
        void onTargetResumedEvent(const Events::TargetExecutionResumed& event);
        void onTargetResetEvent(const Events::TargetReset& event);
        void onTargetRegistersWrittenEvent(const Events::RegistersWrittenToTarget& event);
        void onTargetMemoryWrittenEvent(const Events::MemoryWrittenToTarget& event);
        void onProgrammingModeEnabledEvent(const Events::ProgrammingModeEnabled& event);
        void onProgrammingModeDisabledEvent(const Events::ProgrammingModeDisabled& event);
    };
}
