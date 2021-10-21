#include "InsightWorker.hpp"

#include <QObject>
#include <QTimer>

#include "src/Helpers/Thread.hpp"
#include "src/Logger/Logger.hpp"

using namespace Bloom;
using namespace Bloom::Exceptions;

using Bloom::Targets::TargetState;

InsightWorker::InsightWorker(EventManager& eventManager): eventManager(eventManager) {}

void InsightWorker::queueTask(InsightWorkerTask* task) {
    auto taskQueueLock = this->queuedTasks.acquireLock();
    task->moveToThread(this->thread());
    task->setParent(this);
    this->queuedTasks.getReference().push(task);
    emit this->taskQueued();
}

void InsightWorker::startup() {
    Logger::debug("Starting InsightWorker thread");
    this->eventManager.registerListener(this->eventListener);

    this->eventListener->registerCallbackForEventType<Events::TargetControllerStateReported>(
        std::bind(&InsightWorker::onTargetControllerStateReportedEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::TargetExecutionStopped>(
        std::bind(&InsightWorker::onTargetStoppedEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::TargetExecutionResumed>(
        std::bind(&InsightWorker::onTargetResumedEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::RegistersWrittenToTarget>(
        std::bind(&InsightWorker::onTargetRegistersWrittenEvent, this, std::placeholders::_1)
    );

    this->eventDispatchTimer = new QTimer(this);
    QObject::connect(this->eventDispatchTimer, &QTimer::timeout, this, &InsightWorker::dispatchEvents);
    this->eventDispatchTimer->start(5);

    QObject::connect(this, &InsightWorker::taskQueued, this, &InsightWorker::executeTasks);

    this->eventManager.triggerEvent(
        std::make_shared<Events::InsightThreadStateChanged>(ThreadState::READY)
    );

    emit this->ready();
}

void InsightWorker::requestPinStates(int variantId) {
    this->targetControllerConsole.requestPinStates(variantId);
}

std::optional<InsightWorkerTask*> InsightWorker::getQueuedTask() {
    auto task = std::optional<InsightWorkerTask*>();

    auto& queuedTasks = this->queuedTasks.getReference();
    auto taskQueueLock = this->queuedTasks.acquireLock();

    if (!queuedTasks.empty()) {
        task = queuedTasks.front();
        queuedTasks.pop();
    }

    return task;
}

void InsightWorker::onTargetStoppedEvent(const Events::TargetExecutionStopped& event) {
    /*
     * When we report a target halt to Insight, Insight will immediately seek more data from the target (such as GPIO
     * pin states). This can be problematic for cases where the target had halted due to a conditional breakpoint.
     *
     * For conditional breakpoints, a software breakpoint is employed to halt target execution and give the debug
     * client an opportunity to check the condition. In cases where the condition is not met, the client will
     * immediately request for execution to be resumed. It's important that Insight does not get in the way of this
     * process, as it could end up slowing things down significantly.
     *
     * For the above reason, we don't want to report any target halts to Insight, unless we can be sure that the client
     * isn't going to immediately resume execution upon checking the condition.
     *
     * We do this by providing a time window for the TargetExecutionResumed event. If the event is triggered within
     * that time window, we won't report the target halt to Insight, thus preventing Insight from needlessly seeking
     * data from the target and slowing things down.
     *
     * This isn't the best approach, TBH, as it introduces a delay to Insight's response to the target halting. The
     * problem is, we cannot differentiate a conditional breakpoint with a software breakpoint, so this seems to be the
     * only way. It would be nice if the debug client gave us some form of indication of whether the breakpoint is a
     * conditional one.
     */
    auto resumedEvent = this->eventListener->waitForEvent<Events::TargetExecutionResumed>(
        std::chrono::milliseconds(650)
    );

    if (!resumedEvent.has_value()) {
        emit this->targetStateUpdated(TargetState::STOPPED);
        emit this->targetProgramCounterUpdated(event.programCounter);
    }
}

void InsightWorker::onTargetResumedEvent(const Events::TargetExecutionResumed& event) {
    emit this->targetStateUpdated(TargetState::RUNNING);
}

void InsightWorker::onTargetRegistersWrittenEvent(const Events::RegistersWrittenToTarget& event) {
    emit this->targetRegistersWritten(event.registers, event.createdTimestamp);
}

void InsightWorker::onTargetControllerStateReportedEvent(const Events::TargetControllerStateReported& event) {
    if (this->lastTargetControllerState == TargetControllerState::ACTIVE
        && event.state == TargetControllerState::SUSPENDED
    ) {
        emit this->targetControllerSuspended();

    } else if (this->lastTargetControllerState == TargetControllerState::SUSPENDED
        && event.state == TargetControllerState::ACTIVE
    ) {
        try {
            emit this->targetControllerResumed(this->targetControllerConsole.getTargetDescriptor());

        } catch (const Exception& exception) {
            Logger::error("Insight resume failed - " + exception.getMessage());
        }
    }
    this->lastTargetControllerState = event.state;
}

void InsightWorker::executeTasks() {
    auto task = std::optional<InsightWorkerTask*>();

    while ((task = this->getQueuedTask()).has_value()) {
        task.value()->execute(this->targetControllerConsole);
    }
}
