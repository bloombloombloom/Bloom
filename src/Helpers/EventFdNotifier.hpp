#pragma once

#include <optional>

#include "NotifierInterface.hpp"

namespace Bloom
{
    /**
     * The EventFdNotifier class is an RAII wrapper for a Linux eventfd object.
     *
     * It uses an eventfd object to implement the NotifierInterface.
     */
    class EventFdNotifier: public NotifierInterface
    {
    public:
        EventFdNotifier();

        /*
         * EventNotifier objects should not be copied.
         */
        EventFdNotifier(EventFdNotifier& other) = delete;
        EventFdNotifier& operator = (EventFdNotifier& other) = delete;

        /*
         * TODO: Implement this. For now, use the move constructor.
         */
        EventFdNotifier& operator = (EventFdNotifier&& other) = delete;

        EventFdNotifier(EventFdNotifier&& other) noexcept;
        ~EventFdNotifier() noexcept override;

        [[nodiscard]] int getFileDescriptor() const {
            return this->fileDescriptor.value();
        }

        void notify() override;

        void clear();

    private:
        std::optional<int> fileDescriptor;

        void close();
    };
}
