#include <thread>
#include <filesystem>
#include <typeindex>
#include <QObject>
#include <QTimer>

#include "InsightWorker.hpp"
#include "src/Logger/Logger.hpp"
#include "src/Helpers/Thread.hpp"
#include "src/Exceptions/InvalidConfig.hpp"

using namespace Bloom;
using namespace Bloom::Exceptions;

using Bloom::Targets::TargetState;

void InsightWorker::startup() {
    Logger::debug("Starting InsightWorker thread");
    this->eventManager.registerListener(this->eventListener);

    this->eventListener->registerCallbackForEventType<Events::TargetControllerStateReported>(
        std::bind(&InsightWorker::onTargetControllerStateReported, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::TargetExecutionStopped>(
        std::bind(&InsightWorker::onTargetStoppedEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::TargetExecutionResumed>(
        std::bind(&InsightWorker::onTargetResumedEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::TargetPinStatesRetrieved>(
        std::bind(&InsightWorker::onTargetPinStatesRetrievedEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::TargetIoPortsUpdated>(
        std::bind(&InsightWorker::onTargetIoPortsUpdatedEvent, this, std::placeholders::_1)
    );

    this->eventDispatchTimer = new QTimer(this);
    QTimer::connect(this->eventDispatchTimer, &QTimer::timeout, this, &InsightWorker::dispatchEvents);
    this->eventDispatchTimer->start(5);

    this->eventManager.triggerEvent(
        std::make_shared<Events::InsightThreadStateChanged>(ThreadState::READY)
    );
}

void InsightWorker::requestPinStates(int variantId) {
    this->targetControllerConsole.requestPinStates(variantId);
}

void InsightWorker::requestPinStateUpdate(
    int variantId,
    Bloom::Targets::TargetPinDescriptor pinDescriptor,
    Bloom::Targets::TargetPinState pinState
) {
    this->targetControllerConsole.setPinState(variantId, pinDescriptor, pinState);
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

void InsightWorker::onTargetPinStatesRetrievedEvent(const Events::TargetPinStatesRetrieved& event) {
    emit this->targetPinStatesUpdated(event.variantId, event.pinSatesByNumber);
}

void InsightWorker::onTargetIoPortsUpdatedEvent(const Events::TargetIoPortsUpdated& event) {
    emit this->targetIoPortsUpdated();
}

void InsightWorker::onTargetControllerStateReported(const Events::TargetControllerStateReported& event) {
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
