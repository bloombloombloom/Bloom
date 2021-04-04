#pragma once

#include <QtCore>
#include <QApplication>

#include "UserInterfaces/InsightWindow/InsightWindow.hpp"
#include "src/Helpers/Thread.hpp"
#include "src/ApplicationConfig.hpp"
#include "src/EventManager/EventManager.hpp"
#include "src/EventManager/EventListener.hpp"

namespace Bloom
{
    using namespace Events;
    using Targets::TargetState;

    /**
     * The InsightWorker runs on a separate thread to the main GUI thread. Its purpose is to handle any
     * blocking/time-expensive operations.
     */
    class InsightWorker: public QObject
    {
    Q_OBJECT
    private:
        EventManager& eventManager;
        EventListenerPointer eventListener = std::make_shared<EventListener>("InsightWorkerEventListener");

        QTimer* eventDispatchTimer = nullptr;

        void onTargetStoppedEvent(EventPointer<TargetExecutionStopped> event);
        void onTargetResumedEvent(EventPointer<TargetExecutionResumed> event);
        void onTargetPinStatesRetrievedEvent(EventPointer<TargetPinStatesRetrieved> event);
        void onTargetIoPortsUpdatedEvent(EventPointer<TargetIoPortsUpdated> event);

    public:
        InsightWorker(EventManager& eventManager): eventManager(eventManager) {};

        void dispatchEvents() {
            this->eventListener->dispatchCurrentEvents();
        }

    public slots:
        void startup();
        void requestPinStates(int variantId);
        void requestPinStateUpdate(
            int variantId,
            Bloom::Targets::TargetPinDescriptor pinDescriptor,
            Bloom::Targets::TargetPinState pinState
        );

    signals:
        void targetStateUpdated(Bloom::Targets::TargetState newState);
        void targetProgramCounterUpdated(quint32 programCounter);
        void targetPinStatesUpdated(int variantId, Bloom::Targets::TargetPinStateMappingType pinStatesByNumber);
        void targetIoPortsUpdated();

    };
}
