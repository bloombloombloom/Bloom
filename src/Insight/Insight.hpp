#pragma once

#include <QtCore>
#include <QApplication>
#include <cstdint>
#include <map>
#include <utility>
#include <QThread>

#include "src/Helpers/Thread.hpp"
#include "src/Services/PathService.hpp"
#include "src/ProjectConfig.hpp"
#include "src/ProjectSettings.hpp"

#include "src/EventManager/EventListener.hpp"
#include "src/EventManager/Events/Events.hpp"

#include "src/Services/TargetControllerService.hpp"
#include "src/Targets/TargetState.hpp"

#include "InsightSignals.hpp"

#include "InsightWorker/InsightWorker.hpp"
#include "UserInterfaces/InsightWindow/InsightWindow.hpp"

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
     * Opens main window and obtains focus.
     */
    void activateMainWindow();

    /**
     * Shuts down Insight. Called when the user closes the Insight window or a ShutdownApplication event is fired.
     */
    void shutdown();

private:
    static constexpr std::uint8_t INSIGHT_WORKER_COUNT = 3;

    EventListener& eventListener;

    ProjectConfig projectConfig;
    EnvironmentConfig environmentConfig;
    InsightConfig insightConfig;

    InsightProjectSettings& insightProjectSettings;

    Services::TargetControllerService targetControllerService = {};

    const Targets::TargetDescriptor& targetDescriptor;
    const Targets::TargetState& targetState;

    QString globalStylesheet;

    std::map<decltype(InsightWorker::id), std::pair<InsightWorker*, QThread*>> insightWorkersById;
    InsightWindow* mainWindow = nullptr;

    InsightSignals* insightSignals = InsightSignals::instance();

    void onInsightWindowDestroyed();
    void onTargetStateChangedEvent(const Events::TargetStateChanged& event);
    void onTargetResetEvent(const Events::TargetReset& event);
    void onTargetRegistersWrittenEvent(const Events::RegistersWrittenToTarget& event);
    void onTargetMemoryWrittenEvent(const Events::MemoryWrittenToTarget& event);
};
