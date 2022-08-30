#include "InsightWorker.hpp"

#include <QObject>
#include <QTimer>

#include "src/TargetController/TargetControllerState.hpp"

#include "src/Logger/Logger.hpp"

namespace Bloom
{
    using namespace Bloom::Exceptions;

    using Bloom::Targets::TargetState;

    void InsightWorker::queueTask(InsightWorkerTask* task) {
        auto taskQueueLock = this->queuedTasks.acquireLock();
        task->moveToThread(this->thread());
        task->setParent(this);
        this->queuedTasks.getValue().push(task);
        emit this->taskQueued();
    }

    void InsightWorker::startup() {
        Logger::debug("Starting InsightWorker thread");
        EventManager::registerListener(this->eventListener);

        this->eventListener->registerCallbackForEventType<Events::TargetControllerStateChanged>(
            std::bind(&InsightWorker::onTargetControllerStateChangedEvent, this, std::placeholders::_1)
        );

        this->eventListener->registerCallbackForEventType<Events::TargetExecutionStopped>(
            std::bind(&InsightWorker::onTargetStoppedEvent, this, std::placeholders::_1)
        );

        this->eventListener->registerCallbackForEventType<Events::TargetExecutionResumed>(
            std::bind(&InsightWorker::onTargetResumedEvent, this, std::placeholders::_1)
        );

        this->eventListener->registerCallbackForEventType<Events::TargetReset>(
            std::bind(&InsightWorker::onTargetResetEvent, this, std::placeholders::_1)
        );

        this->eventListener->registerCallbackForEventType<Events::RegistersWrittenToTarget>(
            std::bind(&InsightWorker::onTargetRegistersWrittenEvent, this, std::placeholders::_1)
        );

        this->eventListener->registerCallbackForEventType<Events::ProgrammingModeEnabled>(
            std::bind(&InsightWorker::onProgrammingModeEnabledEvent, this, std::placeholders::_1)
        );

        this->eventListener->registerCallbackForEventType<Events::ProgrammingModeDisabled>(
            std::bind(&InsightWorker::onProgrammingModeDisabledEvent, this, std::placeholders::_1)
        );

        this->eventDispatchTimer = new QTimer(this);
        QObject::connect(this->eventDispatchTimer, &QTimer::timeout, this, &InsightWorker::dispatchEvents);
        this->eventDispatchTimer->start(5);

        QObject::connect(this, &InsightWorker::taskQueued, this, &InsightWorker::executeTasks);

        EventManager::triggerEvent(
            std::make_shared<Events::InsightThreadStateChanged>(ThreadState::READY)
        );

        emit this->ready();
    }

    void InsightWorker::onInsightWindowActivated() {
        this->lastTargetState = this->targetControllerConsole.getTargetState();
        emit this->targetStateUpdated(this->lastTargetState);
    }

    std::optional<InsightWorkerTask*> InsightWorker::getQueuedTask() {
        auto task = std::optional<InsightWorkerTask*>();

        auto& queuedTasks = this->queuedTasks.getValue();
        auto taskQueueLock = this->queuedTasks.acquireLock();

        if (!queuedTasks.empty()) {
            task = queuedTasks.front();
            queuedTasks.pop();
        }

        return task;
    }

    void InsightWorker::onTargetStoppedEvent(const Events::TargetExecutionStopped& event) {
        if (this->lastTargetState == TargetState::STOPPED) {
            return;
        }

        this->lastTargetState = TargetState::STOPPED;
        emit this->targetStateUpdated(TargetState::STOPPED);
        emit this->targetProgramCounterUpdated(event.programCounter);
    }

    void InsightWorker::onTargetResumedEvent(const Events::TargetExecutionResumed& event) {
        if (this->lastTargetState != TargetState::RUNNING) {
            this->lastTargetState = TargetState::RUNNING;
            emit this->targetStateUpdated(TargetState::RUNNING);
        }
    }

    void InsightWorker::onTargetResetEvent(const Events::TargetReset& event) {
        try {
            if (this->targetControllerConsole.getTargetState() != TargetState::STOPPED) {
                return;
            }

            if (this->lastTargetState != TargetState::STOPPED) {
                this->lastTargetState = TargetState::STOPPED;
                emit this->targetStateUpdated(TargetState::STOPPED);
            }

            emit this->targetProgramCounterUpdated(this->targetControllerConsole.getProgramCounter());

        } catch (const Exceptions::Exception& exception) {
            Logger::debug("Error handling TargetReset event - " + exception.getMessage());
        }
    }

    void InsightWorker::onTargetRegistersWrittenEvent(const Events::RegistersWrittenToTarget& event) {
        emit this->targetRegistersWritten(event.registers, event.createdTimestamp);

        for (const auto& reg : event.registers) {
            if (reg.descriptor.type == Targets::TargetRegisterType::PROGRAM_COUNTER) {
                try {
                    emit this->targetProgramCounterUpdated(this->targetControllerConsole.getProgramCounter());

                } catch (const Exceptions::Exception& exception) {
                    Logger::debug("Error reading program counter - " + exception.getMessage());
                }

                break;
            }
        }
    }

    void InsightWorker::onTargetControllerStateChangedEvent(const Events::TargetControllerStateChanged& event) {
        using TargetController::TargetControllerState;

        if (event.state == TargetControllerState::SUSPENDED) {
            emit this->targetControllerSuspended();

        } else if (event.state == TargetControllerState::ACTIVE) {
            try {
                emit this->targetControllerResumed(this->targetControllerConsole.getTargetDescriptor());

            } catch (const Exception& exception) {
                Logger::error("Insight resume failed - " + exception.getMessage());
            }
        }
    }

    void InsightWorker::onProgrammingModeEnabledEvent(const Events::ProgrammingModeEnabled& event) {
        emit this->programmingModeEnabled();
    }

    void InsightWorker::onProgrammingModeDisabledEvent(const Events::ProgrammingModeDisabled& event) {
        emit this->programmingModeDisabled();
    }

    void InsightWorker::executeTasks() {
        auto task = std::optional<InsightWorkerTask*>();

        while ((task = this->getQueuedTask()).has_value()) {
            task.value()->execute(this->targetControllerConsole);
        }
    }
}
