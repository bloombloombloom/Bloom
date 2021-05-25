#pragma once

#include <string>
#include <map>
#include <functional>
#include <queue>
#include <memory>
#include <variant>
#include <optional>
#include <mutex>
#include <iostream>
#include <condition_variable>
#include <set>

#include "src/EventManager/Events/Events.hpp"
#include "src/Logger/Logger.hpp"
#include "src/Helpers/SyncSafe.hpp"
#include "src/Helpers/EventNotifier.hpp"

namespace Bloom
{
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
    private:
        /**
         * Human readable name for event listeners.
         *
         * TODO: This was useful during development, but may no longer be needed.
         */
        std::string name;

        static inline std::atomic<std::size_t> lastId = 0;
        std::size_t id = ++(this->lastId);

        /**
         * Holds all events registered to this listener.
         *
         * Events are grouped by event type name, and removed from their queue just *before* the dispatching to
         * registered handlers begins.
         */
        SyncSafe<std::map<std::string, std::queue<Events::GenericEventPointer>>> eventQueueByEventType;
        std::condition_variable eventQueueByEventTypeCV;

        /**
         * A mapping of event type names to a vector of callback functions. Events will be dispatched to these
         * callback functions, during a call to EventListener::dispatchEvent().
         *
         * Each callback will be passed an std::shared_ptr<const EventType> of the event (we wrap all registered
         * callbacks in a lambda, where we perform a downcast before invoking the callback.
         * See EventListener::registerCallbackForEventType() for more)
         */
        SyncSafe<std::map<std::string, std::vector<std::function<void(Events::GenericEventPointer)>>>> eventTypeToCallbacksMapping;
        SyncSafe<std::set<std::string>> registeredEventTypes;

        std::shared_ptr<EventNotifier> interruptEventNotifier = nullptr;

        std::vector<Events::GenericEventPointer> getEvents();

    public:
        explicit EventListener(const std::string& name): name(name) {};

        std::size_t getId() const {
            return this->id;
        };

        /**
         * Generates a list of event types names currently registered in the listener.
         *
         * @return
         */
        std::set<std::string> getRegisteredEventTypeNames();

        /**
         * Registers an event with the event listener
         *
         * @param event
         */
        void registerEvent(Events::GenericEventPointer event);

        void setInterruptEventNotifier(std::shared_ptr<EventNotifier> interruptEventNotifier) {
            this->interruptEventNotifier = interruptEventNotifier;
        }

        /**
         * Registers a callback function for an event type. The callback function will be
         * invoked upon an event of type EventType being dispatched to the listener.
         *
         * @tparam EventType
         * @param callback
         */
        template<class EventType>
        void registerCallbackForEventType(std::function<void(std::shared_ptr<const EventType>)> callback) {
            // We encapsulate the callback in a lambda to handle the downcasting.
            std::function<void(Events::GenericEventPointer)> parentCallback =
                [callback] (Events::GenericEventPointer event) {
                    // Downcast the event to the expected type
                    callback(std::dynamic_pointer_cast<const EventType>(event));
                }
            ;

            auto mappingLock = this->eventTypeToCallbacksMapping.acquireLock();
            auto& mapping = this->eventTypeToCallbacksMapping.getReference();

            if (mapping.find(EventType::name) == mapping.end()) {
                /*
                 * Multiple callbacks can be registered for a single event type.
                 *
                 * We have no callbacks for this event type registered in this listener, so setup
                 * the type name to callback vector mapping.
                 */
                mapping.insert(
                    std::pair<std::string, std::vector<std::function<void(Events::GenericEventPointer)>>>(
                        EventType::name,
                        std::vector<std::function<void(Events::GenericEventPointer)>>()
                    )
                );
            }

            mapping[EventType::name].push_back(parentCallback);
            auto registeredEventTypesLock = this->registeredEventTypes.acquireLock();
            this->registeredEventTypes.getReference().insert(EventType::name);
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
            std::optional<std::chrono::milliseconds> timeout = std::nullopt,
            std::optional<int> correlationId = std::nullopt
        ) {
            // Different return types, depending on how many event type arguments are passed in.
            using MonoType = std::optional<Events::EventPointer<EventTypeA>>;
            using BiVariantType = std::optional<
                std::variant<
                    std::monostate,
                    Events::EventPointer<EventTypeA>,
                    Events::EventPointer<EventTypeB>
                >
            >;
            using TriVariantType = std::optional<
                std::variant<
                    std::monostate,
                    Events::EventPointer<EventTypeA>,
                    Events::EventPointer<EventTypeB>,
                    Events::EventPointer<EventTypeC>
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

            ReturnType output = std::nullopt;

            auto queueLock = this->eventQueueByEventType.acquireLock();
            auto& eventQueueByType = this->eventQueueByEventType.getReference();

            auto eventTypeNames = std::set<std::string>({EventTypeA::name});
            auto eventTypeNamesToDeRegister = std::set<std::string>();

            if constexpr (!std::is_same_v<EventTypeA, EventTypeB>) {
                static_assert(
                    std::is_base_of_v<Events::Event, EventTypeB>,
                    "All event types must be derived from the Event base class."
                );
                eventTypeNames.insert(EventTypeB::name);
            }

            if constexpr (!std::is_same_v<EventTypeB, EventTypeC>) {
                static_assert(
                    std::is_base_of_v<Events::Event, EventTypeC>,
                    "All event types must be derived from the Event base class."
                );
                eventTypeNames.insert(EventTypeC::name);
            }

            {
                auto registeredEventTypesLock = this->registeredEventTypes.acquireLock();
                auto& registeredEventTypes = this->registeredEventTypes.getReference();

                for (const auto& eventTypeName : eventTypeNames) {
                    if (registeredEventTypes.find(eventTypeName) == registeredEventTypes.end()) {
                        registeredEventTypes.insert(eventTypeName);
                        eventTypeNamesToDeRegister.insert(eventTypeName);
                    }
                }
            }

            Events::GenericEventPointer foundEvent = nullptr;
            auto eventsFound = [&eventTypeNames, &eventQueueByType, &correlationId, &foundEvent]() -> bool {
                for (const auto& eventTypeName : eventTypeNames) {
                    if (eventQueueByType.find(eventTypeName) != eventQueueByType.end()
                        && eventQueueByType.find(eventTypeName)->second.size() > 0
                    ) {
                        auto& queue = eventQueueByType.find(eventTypeName)->second;
                        while (queue.size() > 0) {
                            auto event = queue.front();

                            if (!correlationId.has_value()
                                || (event->correlationId.has_value() && event->correlationId == correlationId)
                            ) {
                                foundEvent = event;
                                queue.pop();
                                return true;
                            }

                            // Events that match in type but not correlation ID are disregarded
                            queue.pop();
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

            if (!eventTypeNamesToDeRegister.empty()) {
                auto registeredEventTypesLock = this->registeredEventTypes.acquireLock();
                auto& registeredEventTypes = this->registeredEventTypes.getReference();

                for (const auto& eventTypeName : eventTypeNamesToDeRegister) {
                    registeredEventTypes.erase(eventTypeName);
                }
            }

            if (foundEvent != nullptr) {
                // If we're looking for multiple event types, use an std::variant.
                if constexpr (!std::is_same_v<EventTypeA, EventTypeB> || !std::is_same_v<EventTypeB, EventTypeC>) {
                    if (foundEvent->getName() == EventTypeA::name) {
                        output = std::optional<typename decltype(output)::value_type>(
                            std::dynamic_pointer_cast<const EventTypeA>(foundEvent)
                        );

                    } else if constexpr (!std::is_same_v<EventTypeA, EventTypeB>) {
                        if (foundEvent->getName() == EventTypeB::name) {
                            output = std::optional<typename decltype(output)::value_type>(
                                std::dynamic_pointer_cast<const EventTypeB>(foundEvent)
                            );
                        }
                    }

                    if constexpr (!std::is_same_v<EventTypeB, EventTypeC>) {
                        if (foundEvent->getName() == EventTypeC::name) {
                            output = std::optional<typename decltype(output)::value_type>(
                                std::dynamic_pointer_cast<const EventTypeC>(foundEvent)
                            );
                        }
                    }

                } else {
                    if (foundEvent->getName() == EventTypeA::name) {
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

        void dispatchEvent(Events::GenericEventPointer event);

        void dispatchCurrentEvents();

        /**
         * Removes all callbacks registered for the event listener.
         */
        void clearAllCallbacks();
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
}
