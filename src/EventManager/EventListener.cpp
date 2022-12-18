#include "EventListener.hpp"

#include "src/Logger/Logger.hpp"

namespace Bloom
{
    using namespace Bloom::Events;

    std::set<Events::EventType> EventListener::getRegisteredEventTypes() {
        const auto lock = this->registeredEventTypes.acquireLock();
        return this->registeredEventTypes.getValue();
    }

    void EventListener::registerEvent(SharedGenericEventPointer event) {
        Logger::debug(
            "Event \"" + event->getName() + "\" (" + std::to_string(event->id) + ") registered for listener "
                + this->name
        );

        const auto queueLock = this->eventQueueByEventType.acquireLock();
        auto& eventQueueByType = this->eventQueueByEventType.getValue();

        eventQueueByType[event->getType()].push(std::move(event));
        this->eventQueueByEventTypeCV.notify_all();

        if (this->interruptEventNotifier != nullptr) {
            this->interruptEventNotifier->notify();
        }
    }

    void EventListener::waitAndDispatch(int msTimeout) {
        {
            auto queueLock = this->eventQueueByEventType.acquireLock();
            const auto& eventQueueByType = this->eventQueueByEventType.getValue();
            const auto registeredEventTypes = this->getRegisteredEventTypes();
            std::optional<SharedGenericEventPointer> event;

            const auto eventsFound = [&registeredEventTypes, &event, &eventQueueByType]() -> bool {
                for (auto& eventQueue: eventQueueByType) {
                    if (registeredEventTypes.contains(eventQueue.first) && !eventQueue.second.empty()) {
                        return true;
                    }
                }
                return false;
            };

            if (msTimeout > 0) {
                this->eventQueueByEventTypeCV.wait_for(queueLock, std::chrono::milliseconds(msTimeout), eventsFound);

            } else {
                this->eventQueueByEventTypeCV.wait(queueLock, eventsFound);
            }
        }

        this->dispatchCurrentEvents();
    }

    void EventListener::dispatchEvent(const SharedGenericEventPointer& event) {
        Logger::debug("Dispatching event " + event->getName() + " (" + std::to_string(event->id) + ").");

        // Dispatch the event to all registered handlers
        auto callbacks = std::vector<std::function<void(const Events::Event&)>>();

        {
            const auto mappingLock = this->eventTypeToCallbacksMapping.acquireLock();
            callbacks = this->eventTypeToCallbacksMapping.getValue().find(event->getType())->second;
        }

        for (auto& callback : callbacks) {
            callback(*(event.get()));
        }
    }

    void EventListener::dispatchCurrentEvents() {
        auto events = this->getEvents();

        for (const auto& event: events) {
            dispatchEvent(event);
        }
    }

    std::vector<SharedGenericEventPointer> EventListener::getEvents() {
        const auto queueLock = this->eventQueueByEventType.acquireLock();
        auto& eventQueueByType = this->eventQueueByEventType.getValue();
        std::vector<SharedGenericEventPointer> output;

        for (auto& eventQueue: eventQueueByType) {
            while (!eventQueue.second.empty()) {
                output.push_back(std::move(eventQueue.second.front()));
                eventQueue.second.pop();
            }
        }

        std::sort(
            output.begin(),
            output.end(),
            [] (const SharedGenericEventPointer& a, const SharedGenericEventPointer& b) {
                return a->id < b->id;
            }
        );

        return output;
    }

    void EventListener::clearAllCallbacks() {
        const auto lock = this->eventTypeToCallbacksMapping.acquireLock();
        this->eventTypeToCallbacksMapping.getValue().clear();
    }
}
