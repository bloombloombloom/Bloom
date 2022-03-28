#pragma once

#include <sys/epoll.h>
#include <cstdint>
#include <optional>
#include <chrono>

namespace Bloom
{
    enum class EpollEvent: std::uint16_t
    {
        READ_READY = EPOLL_EVENTS::EPOLLIN,
        WRITE_READY = EPOLL_EVENTS::EPOLLOUT,
    };

    /**
     * The EpollInstance class is an RAII wrapper for a single Linux epoll instance.
     *
     * See https://man7.org/linux/man-pages/man7/epoll.7.html for more on the Linux epoll API.
     */
    class EpollInstance
    {
    public:
        EpollInstance();

        /**
         * Adds an entry to the epoll instance.
         *
         * @param fileDescriptor
         * @param eventMask
         */
        void addEntry(int fileDescriptor, std::uint16_t eventMask);

        /**
         * Removes an entry from the epoll instance.
         *
         * @param fileDescriptor
         */
        void removeEntry(int fileDescriptor);

        /**
         * Waits on the epoll instance until an event occurs for any of the registered files.
         *
         * @param timeout
         *  Millisecond timeout. If not provided, no timeout will be applied and this function will block until an
         *  event occurs.
         *
         * @return
         *  The file descriptor of the file for which the event occurred, or std::nullopt if a timeout was reached.
         */
        std::optional<int> waitForEvent(std::optional<std::chrono::milliseconds> timeout = std::nullopt);

        /*
         * EpollInstance objects should not be copied.
         */
        EpollInstance(EpollInstance& other) = delete;
        EpollInstance& operator = (EpollInstance& other) = delete;

        /*
         * TODO: Implement this. For now, use the move constructor.
         */
        EpollInstance& operator = (EpollInstance&& other) = delete;

        EpollInstance(EpollInstance&& other) noexcept;
        ~EpollInstance() noexcept;

    private:
        std::optional<int> fileDescriptor;
    };
}
