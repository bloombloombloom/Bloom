#pragma once

#include <map>
#include <mutex>

#include "Events/Events.hpp"
#include "EventListener.hpp"

namespace Bloom
{
    /**
     * The EventManager class provides a method of dispatching events to a set of listeners.
     * A single instance of this class is created in Application class. That instance is then passed by references to
     * all other components in Bloom, that require the ability to trigger events.
     *
     * @TODO: Should this be a static class? As in, all methods and variables declared static. We seem to be
     *        using it in that way. It would save us from having to pass around that single instance by reference.
     *        Something to consider.
     */
    class EventManager
    {
    public:
        /**
         * Registers an EventListener instance with this manager.
         *
         * All EventListener instances must be registered with the EventManager before any events can
         * be dispatched to them.
         *
         * The EventManager possesses partial ownership of the EventListener. This is why we use a shared_ptr here.
         *
         * @param listenerName
         */
        void registerListener(std::shared_ptr<EventListener> listener);

        /**
         * Deregister an EventListener instance.
         *
         * @param listenerId
         *  The ID of the EventListener to deregister. See EventListener::getId();
         */
        void deregisterListener(size_t listenerId);

        /**
         * Dispatches an event to all registered listeners, if they have registered an interest in the event type.
         * See EventListener::registeredEventTypes for more.
         *
         * @param event
         */
        void triggerEvent(const Events::SharedGenericEventPointer& event);

        /**
         * Checks if any registered listener is listening for a particular event type.
         *
         * @param eventType
         * @return
         */
        bool isEventTypeListenedFor(Events::EventType eventType);

    private:
        /**
         * A mapping of listener IDs to registered listeners. Each registered listener is given an interger ID.
         */
        static inline std::map<size_t, std::shared_ptr<EventListener>> registeredListeners;
        static inline std::mutex registerListenerMutex;
    };
}
