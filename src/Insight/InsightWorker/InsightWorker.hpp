#pragma once

#include <QtCore>
#include <QApplication>

#include "src/Helpers/Thread.hpp"
#include "src/Helpers/SyncSafe.hpp"
#include "src/ProjectConfig.hpp"

#include "src/EventManager/EventManager.hpp"
#include "src/EventManager/EventListener.hpp"
#include "src/EventManager/Events/Events.hpp"

#include "src/TargetController/TargetControllerConsole.hpp"

#include "Tasks/InsightWorkerTask.hpp"

namespace Bloom
{
    /**
     * The InsightWorker runs on a separate thread to the main GUI thread. Its purpose is to handle any
     * blocking/time-expensive operations.
     */
    class InsightWorker: public QObject
    {
        Q_OBJECT

    public:
        InsightWorker() = default;

        void queueTask(InsightWorkerTask* task);

        void startup();

        void onInsightWindowActivated();

    signals:
        void ready();
        void taskQueued();
        void targetStateUpdated(Bloom::Targets::TargetState newState);
        void targetReset();
        void targetControllerSuspended();
        void targetControllerResumed(const Bloom::Targets::TargetDescriptor& targetDescriptor);
        void targetRegistersWritten(const Bloom::Targets::TargetRegisters& targetRegisters, const QDateTime& timestamp);
        void programmingModeEnabled();
        void programmingModeDisabled();

    private:
        EventListenerPointer eventListener = std::make_shared<EventListener>("InsightWorkerEventListener");

        TargetController::TargetControllerConsole targetControllerConsole = TargetController::TargetControllerConsole();

        Targets::TargetState lastTargetState = Targets::TargetState::UNKNOWN;

        QTimer* eventDispatchTimer = nullptr;

        SyncSafe<std::queue<InsightWorkerTask*>> queuedTasks;

        void dispatchEvents() {
            this->eventListener->dispatchCurrentEvents();
        }

        std::optional<InsightWorkerTask*> getQueuedTask();

        void onTargetStoppedEvent(const Events::TargetExecutionStopped& event);
        void onTargetResumedEvent(const Events::TargetExecutionResumed& event);
        void onTargetResetEvent(const Events::TargetReset& event);
        void onTargetControllerStateChangedEvent(const Events::TargetControllerStateChanged& event);
        void onProgrammingModeEnabledEvent(const Events::ProgrammingModeEnabled& event);
        void onProgrammingModeDisabledEvent(const Events::ProgrammingModeDisabled& event);

        void executeTasks();
    };
}
