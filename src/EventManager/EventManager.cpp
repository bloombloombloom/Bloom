#include "EventManager.hpp"

using namespace Bloom;

void EventManager::registerListener(std::shared_ptr<EventListener> listener) {
    auto registerListenersLock = std::unique_lock(this->registerListenerMutex);
    this->registeredListeners.insert(std::pair(listener->getId(), listener));
}

void EventManager::deregisterListener(size_t listenerId) {
    auto registerListenersLock = std::unique_lock(this->registerListenerMutex);
    this->registeredListeners.erase(listenerId);
}

void EventManager::triggerEvent(std::shared_ptr<const Events::Event> event) {
    auto registerListenersLock = std::unique_lock(this->registerListenerMutex);
    for(auto const& [listenerId, listener] : this->registeredListeners) {
        auto registeredEventTypes = listener->getRegisteredEventTypeNames();
        if (registeredEventTypes.contains(event->getName())) {
            listener->registerEvent(event);
        }
    }
}
