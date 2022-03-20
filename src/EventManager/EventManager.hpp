#pragma once

#include <map>
#include <mutex>

#include "Events/Events.hpp"
#include "EventListener.hpp"

namespace Bloom
{
    /**
     * The static EventManager class provides a method of dispatching events to a set of listeners.
     */
    class EventManager
    {
    public:
        /**
         * Registers an EventListener instance with the manager.
         *
         * All EventListener instances must be registered with the EventManager before any events can
         * be dispatched to them.
         *
         * The EventManager possesses partial ownership of the EventListener. This is why we use a shared_ptr here.
         *
         * @param listenerName
         */
        static void registerListener(std::shared_ptr<EventListener> listener);

        /**
         * Deregister an EventListener instance.
         *
         * @param listenerId
         *  The ID of the EventListener to deregister. See EventListener::getId();
         */
        static void deregisterListener(size_t listenerId);

        /**
         * Dispatches an event to all registered listeners, if they have registered an interest in the event type.
         * See EventListener::registeredEventTypes for more.
         *
         * @param event
         */
        static void triggerEvent(const Events::SharedGenericEventPointer& event);

        /**
         * Checks if any registered listener is listening for a particular event type.
         *
         * @param eventType
         * @return
         */
        static bool isEventTypeListenedFor(Events::EventType eventType);

    private:
        /**
         * A mapping of listener IDs to registered listeners. Each registered listener is given an interger ID.
         */
        static inline std::map<size_t, std::shared_ptr<EventListener>> registeredListeners;
        static inline std::mutex registerListenerMutex;
    };
}
