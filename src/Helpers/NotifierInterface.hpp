#pragma once

/**
 * The NotifierInterface class describes an interface for notifying different components, within Bloom, of something
 * important that has just happened.
 *
 * It's important to note that this interface only describes the issuing of notifications. It *does not* describe
 * the listening for notifications. The listening can be defined by the implementation.
 *
 * For example, consider the EventFdNotifier implementation. That class is just an RAII wrapper for a Linux eventfd
 * object. Notifications are recorded by incrementing the eventfd counter. And they can be listened for, using
 * Linux system functions like poll(), select(), and similar.
 *
 * The EventListener class can hold a pointer to a NotifierInterface, where it will invoke
 * NotifierInterface::notify() everytime a new event is registered on the listener.
 */
class NotifierInterface
{
public:
    NotifierInterface() = default;
    virtual ~NotifierInterface() noexcept = default;

    NotifierInterface(NotifierInterface& other) = delete;

    NotifierInterface& operator = (NotifierInterface& other) = delete;
    NotifierInterface& operator = (NotifierInterface&& other) = delete;

    NotifierInterface(NotifierInterface&& other) noexcept = default;

    /**
     * Should record a notification.
     */
    virtual void notify() = 0;
};
