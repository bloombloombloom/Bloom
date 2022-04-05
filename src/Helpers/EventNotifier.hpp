#pragma once

#include <optional>

namespace Bloom
{
    /**
     * The EventNotifier class is an RAII wrapper for a Linux eventfd object.
     *
     * The EventListener can hold an instance to EventNotifier, where it will invoke EventNotifier::notify() everytime
     * a new event is registered on the listener.
     */
    class EventNotifier
    {
    public:
        EventNotifier();

        /*
         * EpollInstance objects should not be copied.
         */
        EventNotifier(EventNotifier& other) = delete;
        EventNotifier& operator = (EventNotifier& other) = delete;

        /*
         * TODO: Implement this. For now, use the move constructor.
         */
        EventNotifier& operator = (EventNotifier&& other) = delete;

        EventNotifier(EventNotifier&& other) noexcept;
        ~EventNotifier() noexcept;

        [[nodiscard]] int getFileDescriptor() const {
            return this->fileDescriptor.value();
        }

        void notify();

        void clear();

    private:
        std::optional<int> fileDescriptor;

        void close();
    };
}
