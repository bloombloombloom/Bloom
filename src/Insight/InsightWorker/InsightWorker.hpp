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
#include "src/TargetController/TargetControllerState.hpp"

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

        void dispatchEvents() {
            this->eventListener->dispatchCurrentEvents();
        }

        void startup();

    signals:
        void ready();
        void taskQueued();
        void targetStateUpdated(Bloom::Targets::TargetState newState);
        void targetProgramCounterUpdated(quint32 programCounter);
        void targetControllerSuspended();
        void targetControllerResumed(const Bloom::Targets::TargetDescriptor& targetDescriptor);
        void targetRegistersWritten(const Bloom::Targets::TargetRegisters& targetRegisters, const QDateTime& timestamp);

    private:
        EventListenerPointer eventListener = std::make_shared<EventListener>("InsightWorkerEventListener");

        TargetController::TargetControllerConsole targetControllerConsole = TargetController::TargetControllerConsole(
            *(this->eventListener)
        );
        TargetController::TargetControllerState lastTargetControllerState =
            TargetController::TargetControllerState::ACTIVE;

        Targets::TargetState lastTargetState = Targets::TargetState::UNKNOWN;

        QTimer* eventDispatchTimer = nullptr;

        SyncSafe<std::queue<InsightWorkerTask*>> queuedTasks;

        std::optional<InsightWorkerTask*> getQueuedTask();

        void onTargetStoppedEvent(const Events::TargetExecutionStopped& event);
        void onTargetResumedEvent(const Events::TargetExecutionResumed& event);
        void onTargetResetEvent(const Events::TargetReset& event);
        void onTargetRegistersWrittenEvent(const Events::RegistersWrittenToTarget& event);
        void onTargetControllerStateReportedEvent(const Events::TargetControllerStateReported& event);

        void executeTasks();
    };
}
