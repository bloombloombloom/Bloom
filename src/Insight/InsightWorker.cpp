#include <thread>
#include <filesystem>
#include <typeindex>
#include <QObject>
#include <QTimer>

#include "src/Application.hpp"
#include "InsightWorker.hpp"
#include "src/Logger/Logger.hpp"
#include "src/Helpers/Thread.hpp"
#include "src/Exceptions/InvalidConfig.hpp"

using namespace Bloom;
using namespace Exceptions;

void InsightWorker::startup() {
    Logger::debug("Starting InsightWorker thread");
    this->eventManager.registerListener(this->eventListener);

    this->eventListener->registerCallbackForEventType<TargetExecutionStopped>(
        std::bind(&InsightWorker::onTargetStoppedEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<TargetExecutionResumed>(
        std::bind(&InsightWorker::onTargetResumedEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<TargetPinStatesRetrieved>(
        std::bind(&InsightWorker::onTargetPinStatesRetrievedEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<TargetIoPortsUpdated>(
        std::bind(&InsightWorker::onTargetIoPortsUpdatedEvent, this, std::placeholders::_1)
    );

    this->eventDispatchTimer = new QTimer(this);
    QTimer::connect(this->eventDispatchTimer, &QTimer::timeout, this, &InsightWorker::dispatchEvents);
    this->eventDispatchTimer->start(5);

    this->eventManager.triggerEvent(
        std::make_shared<Events::InsightStateChanged>(ThreadState::READY)
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

void InsightWorker::onTargetStoppedEvent(EventPointer<TargetExecutionStopped> event) {
    auto resumedEvent = this->eventListener->waitForEvent<TargetExecutionResumed>(std::chrono::milliseconds(650));

    if (!resumedEvent.has_value()) {
        emit this->targetStateUpdated(TargetState::STOPPED);
        emit this->targetProgramCounterUpdated(event->programCounter);
    }
}

void InsightWorker::onTargetResumedEvent(EventPointer<TargetExecutionResumed> event) {
    emit this->targetStateUpdated(TargetState::RUNNING);
}

void InsightWorker::onTargetPinStatesRetrievedEvent(EventPointer<TargetPinStatesRetrieved> event) {
    emit this->targetPinStatesUpdated(event->variantId, event->pinSatesByNumber);
}

void InsightWorker::onTargetIoPortsUpdatedEvent(EventPointer<TargetIoPortsUpdated> event) {
    emit this->targetIoPortsUpdated();
}


