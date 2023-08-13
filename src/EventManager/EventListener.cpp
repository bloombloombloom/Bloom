#include "EventListener.hpp"

#include "src/Logger/Logger.hpp"

using namespace Events;

std::set<Events::EventType> EventListener::getRegisteredEventTypes() {
    return *(this->registeredEventTypes.accessor());
}

void EventListener::registerEvent(SharedGenericEventPointer event) {
    Logger::debug(
        "Event \"" + event->getName() + "\" (" + std::to_string(event->id) + ") registered for listener "
            + this->name
    );

    auto eventQueueByTypeAccessor = this->eventQueueByEventType.accessor();
    auto& eventQueueByType = *(eventQueueByTypeAccessor);

    eventQueueByType[event->getType()].push(std::move(event));
    this->eventQueueByEventTypeCV.notify_all();

    if (this->interruptEventNotifier != nullptr) {
        this->interruptEventNotifier->notify();
    }
}

void EventListener::waitAndDispatch(int msTimeout) {
    {
        auto queueLock = this->eventQueueByEventType.lock();
        const auto& eventQueueByType = this->eventQueueByEventType.unsafeReference();

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
        const auto callbackMappingAccessor = this->eventTypeToCallbacksMapping.accessor();

        const auto callbacksIt = callbackMappingAccessor->find(event->getType());
        if (callbacksIt != callbackMappingAccessor->end()) {
            callbacks = callbacksIt->second;
        }
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
    auto eventQueueByType = this->eventQueueByEventType.accessor();
    std::vector<SharedGenericEventPointer> output;

    for (auto& eventQueue: *eventQueueByType) {
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
    this->eventTypeToCallbacksMapping.accessor()->clear();
}
