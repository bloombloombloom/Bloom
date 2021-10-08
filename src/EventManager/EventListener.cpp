#include "EventListener.hpp"

#include "src/Logger/Logger.hpp"

using namespace Bloom;
using namespace Bloom::Events;

std::set<Events::EventType> EventListener::getRegisteredEventTypes() {
    return this->registeredEventTypes.getValue();
}

void EventListener::registerEvent(SharedGenericEventPointer event) {
    Logger::debug("Event \"" + event->getName() + "\" (" + std::to_string(event->id)
        + ") registered for listener " + this->name);
    auto queueLock = this->eventQueueByEventType.acquireLock();
    auto& eventQueueByType = this->eventQueueByEventType.getReference();

    eventQueueByType[event->getType()].push(std::move(event));
    this->eventQueueByEventTypeCV.notify_all();

    if (this->interruptEventNotifier != nullptr && this->interruptEventNotifier->isInitialised()) {
        this->interruptEventNotifier->notify();
    }
}

void EventListener::waitAndDispatch(int msTimeout) {
    auto queueLock = this->eventQueueByEventType.acquireLock();
    auto& eventQueueByType = this->eventQueueByEventType.getReference();
    auto registeredEventTypes = this->getRegisteredEventTypes();
    std::optional<SharedGenericEventPointer> event;

    auto eventsFound = [&registeredEventTypes, &event, &eventQueueByType] () -> bool {
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

    /*
     * We don't want the dispatch to block other threads from registering more events. We don't need the
     * lock anymore so it's fine to release it here.
     */
    queueLock.unlock();

    this->dispatchCurrentEvents();
}

void EventListener::dispatchEvent(const SharedGenericEventPointer& event) {
    Logger::debug("Dispatching event " + event->getName() + " (" + std::to_string(event->id) + ").");
    // Dispatch the event to all registered handlers
    auto mappingLock = this->eventTypeToCallbacksMapping.acquireLock();
    auto& callbacks = this->eventTypeToCallbacksMapping.getReference().find(event->getType())->second;
    mappingLock.unlock();

    for (auto& callback : callbacks) {
        callback(*(event.get()));
    }
}

void EventListener::dispatchCurrentEvents() {
    auto events = this->getEvents();

    for (auto const& event: events) {
        dispatchEvent(event);
    }
}

std::vector<SharedGenericEventPointer> EventListener::getEvents() {
    auto queueLock = this->eventQueueByEventType.acquireLock();
    auto& eventQueueByType = this->eventQueueByEventType.getReference();
    std::vector<SharedGenericEventPointer> output;

    for (auto& eventQueue: eventQueueByType) {
        if (!eventQueue.second.empty()) {
            output.push_back(std::move(eventQueue.second.front()));
            eventQueue.second.pop();
        }
    }

    std::sort(output.begin(), output.end(), [] (const SharedGenericEventPointer& a, const SharedGenericEventPointer& b) {
        return a->id < b->id;
    });

    return output;
}

void EventListener::clearAllCallbacks() {
    auto lock = this->eventTypeToCallbacksMapping.acquireLock();
    this->eventTypeToCallbacksMapping.getReference().clear();
}
