#pragma once

#include <sys/eventfd.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "src/Exceptions/Exception.hpp"

namespace Bloom
{
    /**
     * The EventNotifier class provides a means to interrupt a thread that is blocked by an IO call.
     *
     * It's implementation is rather simple: It uses a Linux event file descriptor (sys/eventfd.h), which should be used
     * along side epoll() to interrupt a blocking call that is waiting on the epoll file descriptor.
     *
     * The EventListener can hold an instance to EventNotifier, where it will invoke EventNotifier::notify() everytime
     * a new event is registered on the listener.
     *
     * @TODO: This could do with some cleaning. It's a bit hacky. Also, maybe add the ability to register the event
     *        file descriptor to an epoll instance within a public method (instead of relying on the caller to do this
     *        themselves via EventNotifier::getFileDescriptor()).
     */
    class EventNotifier
    {
    private:
        int fileDescriptor = -1;

    public:
        EventNotifier() = default;

        void init() {
            this->fileDescriptor = ::eventfd(0, EFD_NONBLOCK);

            if (this->fileDescriptor < -1) {
                throw Exceptions::Exception("Failed to create new eventfd object - error number: "
                    + std::to_string(errno));
            }
        }

        int getFileDescriptor() {
            return this->fileDescriptor;
        }

        void notify() {
            if (::eventfd_write(this->fileDescriptor, 1) < 0) {
                throw Exceptions::Exception("Failed to increment eventfd counter - error number: "
                    + std::to_string(errno));
            }
        }

        void clear() {
            eventfd_t counter;
            if (::eventfd_read(this->fileDescriptor, &counter) < 0 && errno != EAGAIN) {
                throw Exceptions::Exception("Failed to clear EventNotifier object - eventfd_read failed - "
                    "error number: " + std::to_string(errno));
            }
        }

        void close() {
            ::close(this->fileDescriptor);
        }
    };
}