#include "EventManager.hpp"

using namespace Bloom;

void EventManager::registerListener(std::shared_ptr<EventListener> listener) {
    auto registerListenersLock = std::unique_lock(this->registerListenerMutex);
    this->registeredListeners.insert(std::pair(listener->getId(), std::move(listener)));
}

void EventManager::deregisterListener(size_t listenerId) {
    auto registerListenersLock = std::unique_lock(this->registerListenerMutex);
    this->registeredListeners.erase(listenerId);
}

void EventManager::triggerEvent(const std::shared_ptr<const Events::Event>& event) {
    auto registerListenersLock = std::unique_lock(this->registerListenerMutex);

    for(auto const& [listenerId, listener] : this->registeredListeners) {
        if (listener->isEventTypeRegistered(event->getType())) {
            listener->registerEvent(event);
        }
    }
}
