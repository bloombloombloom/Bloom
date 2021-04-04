#pragma once

#include <string>
#include <map>
#include <mutex>
#include "Events/Events.hpp"
#include "EventListener.hpp"

namespace Bloom
{
    class EventManager
    {
    private:
        /**
         * A mapping of listener IDs to registered listeners. Each registered listener is given an interger ID.
         */
        std::map<size_t, std::shared_ptr<EventListener>> registeredListeners;
        std::mutex registerListenerMutex;

    public:
        /**
         * Generates a new registered listener.
         *
         * @param listenerName
         */
        void registerListener(std::shared_ptr<EventListener> listener);
        void deregisterListener(size_t listenerId);

        void triggerEvent(GenericEventPointer event);
    };

}
