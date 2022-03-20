#include "EventManager.hpp"

namespace Bloom
{
    void EventManager::registerListener(std::shared_ptr<EventListener> listener) {
        auto registerListenersLock = std::unique_lock(EventManager::registerListenerMutex);
        EventManager::registeredListeners.insert(std::pair(listener->getId(), std::move(listener)));
    }

    void EventManager::deregisterListener(size_t listenerId) {
        auto registerListenersLock = std::unique_lock(EventManager::registerListenerMutex);
        EventManager::registeredListeners.erase(listenerId);
    }

    void EventManager::triggerEvent(const std::shared_ptr<const Events::Event>& event) {
        auto registerListenersLock = std::unique_lock(EventManager::registerListenerMutex);

        for (const auto&[listenerId, listener] : EventManager::registeredListeners) {
            if (listener->isEventTypeRegistered(event->getType())) {
                listener->registerEvent(event);
            }
        }
    }

    bool EventManager::isEventTypeListenedFor(Events::EventType eventType) {
        auto registerListenersLock = std::unique_lock(EventManager::registerListenerMutex);

        for (const auto& [listenerId, listener] : EventManager::registeredListeners) {
            if (listener->isEventTypeRegistered(eventType)) {
                return true;
            }
        }

        return false;
    }
}
