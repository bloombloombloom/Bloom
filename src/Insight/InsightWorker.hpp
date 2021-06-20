#pragma once

#include <QtCore>
#include <QApplication>

#include "UserInterfaces/InsightWindow/InsightWindow.hpp"
#include "src/Helpers/Thread.hpp"
#include "src/ApplicationConfig.hpp"
#include "src/EventManager/EventManager.hpp"
#include "src/EventManager/EventListener.hpp"
#include "src/TargetController/TargetControllerConsole.hpp"
#include "src/TargetController/TargetControllerState.hpp"

namespace Bloom
{
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

        TargetControllerConsole targetControllerConsole = TargetControllerConsole(
            this->eventManager,
            *(this->eventListener)
        );

        TargetControllerState lastTargetControllerState = TargetControllerState::ACTIVE;

        QTimer* eventDispatchTimer = nullptr;

        void onTargetStoppedEvent(Events::EventPointer<Events::TargetExecutionStopped> event);
        void onTargetResumedEvent(Events::EventPointer<Events::TargetExecutionResumed> event);
        void onTargetPinStatesRetrievedEvent(Events::EventPointer<Events::TargetPinStatesRetrieved> event);
        void onTargetIoPortsUpdatedEvent(Events::EventPointer<Events::TargetIoPortsUpdated> event);
        void onTargetControllerStateReported(Events::EventPointer<Events::TargetControllerStateReported> event);

    public:
        explicit InsightWorker(EventManager& eventManager): eventManager(eventManager) {};

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
        void targetControllerSuspended();
        void targetControllerResumed(const Bloom::Targets::TargetDescriptor& targetDescriptor);

    };
}
