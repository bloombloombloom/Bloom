#include <sys/eventfd.h>
#include "EventListener.hpp"

using namespace Bloom;

std::set<std::string> EventListener::getRegisteredEventTypeNames() {
    return this->registeredEventTypes.getValue();
}

void EventListener::clearAllCallbacks() {
    auto lock = this->eventTypeToCallbacksMapping.acquireLock();
    this->eventTypeToCallbacksMapping.getReference().clear();
}

void EventListener::registerEvent(GenericEventPointer event) {
    auto eventName = event->getName();
    Logger::debug("Event \"" + eventName + "\" (" + std::to_string(event->id)
    + ") registered for listener " + std::to_string(this->id));
    auto queueLock = this->eventQueueByEventType.acquireLock();
    auto& eventQueueByType = this->eventQueueByEventType.getReference();

    if (!eventQueueByType.contains(eventName)) {
        eventQueueByType.insert(
            std::pair<std::string, std::queue<GenericEventPointer>>(
                eventName,
                std::queue<GenericEventPointer>()
            )
        );
    }

    eventQueueByType[eventName].push(event);
    this->eventQueueByEventTypeCV.notify_all();

    if (this->interruptEventNotifier != nullptr) {
        this->interruptEventNotifier->notify();
    }
}

std::vector<GenericEventPointer> EventListener::getEvents() {
    auto queueLock = this->eventQueueByEventType.acquireLock();
    auto& eventQueueByType = this->eventQueueByEventType.getReference();
    std::vector<GenericEventPointer> output;

    for (auto& eventQueue: eventQueueByType) {
        if (eventQueue.second.size() > 0) {
            output.push_back(eventQueue.second.front());
            eventQueue.second.pop();
        }
    }

    std::sort(output.begin(), output.end(), [](GenericEventPointer a, GenericEventPointer b) {
        return a->id < b->id;
    });

    return output;
}

void EventListener::dispatchEvent(GenericEventPointer event) {
    auto eventName = event->getName();
    Logger::debug("Dispatching event " + eventName + ".");
    // Dispatch the event to all registered handlers
    auto mappingLock = this->eventTypeToCallbacksMapping.acquireLock();
    auto& callbacks = this->eventTypeToCallbacksMapping.getReference().find(eventName)->second;
    mappingLock.unlock();

    for (auto& callback : callbacks) {
        callback(event);
    }
}

void EventListener::dispatchCurrentEvents() {
    auto events = this->getEvents();

    for (auto const& event: events) {
        dispatchEvent(event);
    }
}

void EventListener::waitAndDispatch(int msTimeout) {
    auto queueLock = this->eventQueueByEventType.acquireLock();
    auto& eventQueueByType = this->eventQueueByEventType.getReference();
    auto registeredEventTypes = this->getRegisteredEventTypeNames();
    std::optional<GenericEventPointer> event;

    auto eventsFound = [&registeredEventTypes, &event, &eventQueueByType]() -> bool {
        for (auto& eventQueue: eventQueueByType) {
            if (registeredEventTypes.find(eventQueue.first) != registeredEventTypes.end()
                && eventQueue.second.size() > 0
            ) {
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

    /*
     * We don't want the dispatch to block other threads from registering more events. We don't need the
     * lock anymore so it's fine to release it here.
     */
    queueLock.unlock();

    this->dispatchCurrentEvents();
}
