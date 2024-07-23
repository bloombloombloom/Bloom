#pragma once

#include <string>
#include <map>
#include <functional>
#include <queue>
#include <memory>
#include <utility>
#include <variant>
#include <optional>
#include <mutex>
#include <iostream>
#include <condition_variable>
#include <set>

#include "src/EventManager/Events/Events.hpp"
#include "src/Helpers/Synchronised.hpp"
#include "src/Helpers/NotifierInterface.hpp"

/**
 * The EventListener allows specific threads the ability to handle any events, from other threads, that
 * are of interest.
 *
 * Usage is fairly simple:
 *  - Thread A creates an instance to EventListener.
 *  - Thread A registers callbacks for specific event types via EventListener::registerCallbackForEventType().
 *  - Thread A waits for events via EventListener::waitAndDispatch() (or similar methods).
 *  - Thread B triggers an event of a type that Thread A registered a callback for.
 *  - Thread A is woken and the triggered event is dispatched to the registered callbacks for the event type.
 *
 * Events are distributed with shared pointers to const event objects, as each registered handler will own
 * the memory (but should never make any changes to it, hence the const state). Once the final handler has
 * processed the event, the shared pointer reference count will reach 0 and the event object will be destroyed.
 * The type of object within the shared pointers will match that of the specific event type, once it reaches the
 * callback functions. We do this by downcasting the events before we dispatch them to the callback functions.
 *
 * @TODO Whilst event managing should be thread safe, the same cannot be said for all of the event types.
 *       We need to ensure that all event types are thread safe.
 */
class EventListener
{
public:
    explicit EventListener(std::string name)
        : name(std::move(name))
    {};

    std::size_t getId() const {
        return this->id;
    };

    /**
     * Generates a list of event types currently registered in the listener.
     *
     * @return
     */
    std::set<Events::EventType> getRegisteredEventTypes();

    bool isEventTypeRegistered(Events::EventType eventType) {
        return this->registeredEventTypes.accessor()->contains(eventType);
    }

    /**
     * Registers an event type for the listener.
     *
     * Any events of EventType that are triggered from the point of calling this function, will be stored in
     * listener queue until they are dispatched to a callback, or retrieved via a call to this->waitForEvent() or
     * similar.
     *
     * @tparam EventType
     */
    template<class EventType>
    void registerEventType() {
        this->registeredEventTypes.accessor()->insert(EventType::type);
    }

    template<class EventType>
    void deRegisterEventType() {
        this->registeredEventTypes.accessor()->erase(EventType::type);
    }

    /**
     * Registers an event with the event listener
     *
     * @param event
     */
    void registerEvent(Events::SharedGenericEventPointer event);

    void setInterruptEventNotifier(NotifierInterface* interruptEventNotifier) {
        this->interruptEventNotifier = interruptEventNotifier;
    }

    [[nodiscard]] NotifierInterface* getInterruptEventNotifier() {
        return this->interruptEventNotifier;
    }

    /**
     * Registers a callback function for an event type. The callback function will be
     * invoked upon an event of type EventType being dispatched to the listener.
     *
     * @tparam EventType
     * @param callback
     */
    template<class EventType>
    void registerCallbackForEventType(std::function<void(const EventType&)> callback) {
        // We encapsulate the callback in a lambda to handle the downcasting.
        std::function<void(const Events::Event&)> parentCallback =
            [callback] (const Events::Event& event) {
                // Downcast the event to the expected type
                callback(dynamic_cast<const EventType&>(event));
            }
        ;

        auto mappingAccessor = this->eventTypeToCallbacksMapping.accessor();
        auto& mapping = *(mappingAccessor);

        mapping[EventType::type].push_back(parentCallback);
        this->template registerEventType<EventType>();
    }

    /**
     * Clears all registered callbacks for a specific event type.
     *
     * @tparam EventType
     */
    template<class EventType>
    void deregisterCallbacksForEventType() {
        static_assert(
            std::is_base_of<Events::Event, EventType>::value,
            "EventType is not a derivation of Event"
        );

        {
            auto mappingAccessor = this->eventTypeToCallbacksMapping.accessor();
            auto& mapping = *(mappingAccessor);

            if (mapping.contains(EventType::type)) {
                mapping.at(EventType::type).clear();
            }
        }

        this->registeredEventTypes.accessor()->erase(EventType::type);

        auto eventQueueByType = this->eventQueueByEventType.accessor();
        if (eventQueueByType->contains(EventType::type)) {
            eventQueueByType->erase(EventType::type);
        }
    }

    /**
     * Waits for an event (of type EventTypeA, EventTypeB or EventTypeC) to be dispatched to the listener.
     * Then returns the event object. If timeout is reached, an std::nullopt object will be returned.
     *
     * @tparam EventType
     * @param timeout
     *  Millisecond duration to wait for an event to be dispatched to the listener.
     *  A value of std::nullopt will disable the timeout, meaning the function will block until the appropriate
     *  event has been dispatched.
     *
     * @param correlationId
     *  If a correlation ID is provided, this function will ignore any events that do not contain a matching
     *  correlation ID.
     *
     * @return
     *  If only one event type is passed (EventTypeA), an std::optional will be returned, carrying an
     *  event pointer to that event type (or std::nullopt if timeout was reached). If numerous event types are
     *  passed, an std::optional will carry an std::variant of the event pointers to the passed event types
     *  (or std::nullopt if timeout was reached).
     */
    template<class EventTypeA, class EventTypeB = EventTypeA, class EventTypeC = EventTypeB>
    auto waitForEvent(
        std::optional<std::chrono::milliseconds> timeout = std::nullopt
    ) {
        // Different return types, depending on how many event type arguments are passed in.
        using MonoType = std::optional<Events::SharedEventPointer<EventTypeA>>;
        using BiVariantType = std::optional<
            std::variant<
                std::monostate,
                Events::SharedEventPointer<EventTypeA>,
                Events::SharedEventPointer<EventTypeB>
            >
        >;
        using TriVariantType = std::optional<
            std::variant<
                std::monostate,
                Events::SharedEventPointer<EventTypeA>,
                Events::SharedEventPointer<EventTypeB>,
                Events::SharedEventPointer<EventTypeC>
            >
        >;
        using ReturnType = typename std::conditional<
            !std::is_same_v<EventTypeA, EventTypeB> && !std::is_same_v<EventTypeB, EventTypeC>,
            TriVariantType,
            typename std::conditional<!std::is_same_v<EventTypeA, EventTypeB>,
                BiVariantType,
                MonoType
            >::type
        >::type;

        auto output = ReturnType{};

        auto queueLock = this->eventQueueByEventType.lock();
        auto& eventQueueByType = this->eventQueueByEventType.unsafeReference();

        auto eventTypes = std::set<Events::EventType>({EventTypeA::type});
        auto eventTypesToDeRegister = std::set<Events::EventType>{};

        if constexpr (!std::is_same_v<EventTypeA, EventTypeB>) {
            static_assert(
                std::is_base_of_v<Events::Event, EventTypeB>,
                "All event types must be derived from the Event base class."
            );
            eventTypes.insert(EventTypeB::type);
        }

        if constexpr (!std::is_same_v<EventTypeB, EventTypeC>) {
            static_assert(
                std::is_base_of_v<Events::Event, EventTypeC>,
                "All event types must be derived from the Event base class."
            );
            eventTypes.insert(EventTypeC::type);
        }

        {
            auto registeredEventTypes = this->registeredEventTypes.accessor();

            for (const auto& eventType : eventTypes) {
                if (!registeredEventTypes->contains(eventType)) {
                    registeredEventTypes->insert(eventType);
                    eventTypesToDeRegister.insert(eventType);
                }
            }
        }

        Events::SharedGenericEventPointer foundEvent = nullptr;
        auto eventsFound = [&eventTypes, &eventQueueByType, &foundEvent] () -> bool {
            for (const auto& eventType : eventTypes) {
                if (eventQueueByType.find(eventType) != eventQueueByType.end()
                    && !eventQueueByType.find(eventType)->second.empty()
                ) {
                    auto& queue = eventQueueByType.find(eventType)->second;
                    while (!queue.empty()) {
                        foundEvent = queue.front();
                        queue.pop();
                        return true;
                    }
                }
            }

            return false;
        };

        if (timeout.has_value()) {
            this->eventQueueByEventTypeCV.wait_for(queueLock, timeout.value(), eventsFound);

        } else {
            this->eventQueueByEventTypeCV.wait(queueLock, eventsFound);
        }

        if (!eventTypesToDeRegister.empty()) {
            auto registeredEventTypes = this->registeredEventTypes.accessor();

            for (const auto& eventType : eventTypesToDeRegister) {
                registeredEventTypes->erase(eventType);
            }
        }

        if (foundEvent != nullptr) {
            // If we're looking for multiple event types, use an std::variant.
            if constexpr (!std::is_same_v<EventTypeA, EventTypeB> || !std::is_same_v<EventTypeB, EventTypeC>) {
                if (foundEvent->getType() == EventTypeA::type) {
                    output = std::optional<typename decltype(output)::value_type>{
                        std::dynamic_pointer_cast<const EventTypeA>(foundEvent)
                    };

                } else if constexpr (!std::is_same_v<EventTypeA, EventTypeB>) {
                    if (foundEvent->getType() == EventTypeB::type) {
                        output = std::optional<typename decltype(output)::value_type>{
                            std::dynamic_pointer_cast<const EventTypeB>(foundEvent)
                        };
                    }
                }

                if constexpr (!std::is_same_v<EventTypeB, EventTypeC>) {
                    if (foundEvent->getType() == EventTypeC::type) {
                        output = std::optional<typename decltype(output)::value_type>{
                            std::dynamic_pointer_cast<const EventTypeC>(foundEvent)
                        };
                    }
                }

            } else {
                if (foundEvent->getType() == EventTypeA::type) {
                    output = std::dynamic_pointer_cast<const EventTypeA>(foundEvent);
                }
            }
        }

        return output;
    }

    /**
     * Waits for new events with types that have been registered with registerCallbackForEventType() and dispatches
     * any events to their appropriate registered callback handlers.
     *
     * This method will return after one event has been handled.
     */
    void waitAndDispatch(int msTimeout = 0);

    void dispatchEvent(const Events::SharedGenericEventPointer& event);

    void dispatchCurrentEvents();

    /**
     * Removes all callbacks registered for the event listener.
     */
    void clearAllCallbacks();

private:
    /**
     * Human readable name for event listeners.
     *
     * TODO: This was useful during development, but may no longer be needed.
     */
    std::string name;

    static inline std::atomic<std::size_t> lastId = 0;
    std::size_t id = ++(EventListener::lastId);

    /**
     * Holds all events registered to this listener.
     *
     * Events are grouped by event type, and removed from their queue just *before* the dispatching to
     * registered handlers begins.
     */
    Synchronised<std::map<Events::EventType, std::queue<Events::SharedGenericEventPointer>>> eventQueueByEventType;
    std::condition_variable eventQueueByEventTypeCV;

    /**
     * A mapping of event types to a vector of callback functions. Events will be dispatched to these
     * callback functions, during a call to EventListener::dispatchEvent().
     *
     * Each callback will be passed a reference to the event (we wrap all registered callbacks in a lambda, where
     * we perform a downcast before invoking the callback. See EventListener::registerCallbackForEventType()
     * for more)
     */
    Synchronised<
        std::map<Events::EventType, std::vector<std::function<void(const Events::Event&)>>>
    > eventTypeToCallbacksMapping;
    Synchronised<std::set<Events::EventType>> registeredEventTypes;

    NotifierInterface* interruptEventNotifier = nullptr;

    std::vector<Events::SharedGenericEventPointer> getEvents();
};

/**
 * Every component within Bloom that requires access to events will possess an instance to the EventListener class.
 * At some point (usually at component initialisation), the event listener will be registered with the EventManager,
 * using EventManager::registerListener(). Upon registering the listener, the EventManager obtains partial ownership
 * of the event listener.
 *
 * In other words, event listeners are managed by numerous entities and that's why we use a shared_ptr here.
 */
using EventListenerPointer = std::shared_ptr<EventListener>;
