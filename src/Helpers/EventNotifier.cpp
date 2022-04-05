#include "EventNotifier.hpp"

#include <sys/eventfd.h>
#include <unistd.h>
#include <cerrno>
#include <string>

#include "src/Exceptions/Exception.hpp"

namespace Bloom
{
    using Exceptions::Exception;

    EventNotifier::EventNotifier() {
        this->fileDescriptor = ::eventfd(0, EFD_NONBLOCK);

        if (this->fileDescriptor < 0) {
            throw Exception(
                "Failed to create eventfd object - error number " + std::to_string(errno)
                    + " returned."
            );
        }
    }

    EventNotifier::EventNotifier(EventNotifier&& other) noexcept
        : fileDescriptor(other.fileDescriptor)
    {
        other.fileDescriptor = std::nullopt;
    }

    EventNotifier::~EventNotifier() noexcept {
        this->close();
    }

    void EventNotifier::notify() {
        if (::eventfd_write(this->fileDescriptor.value(), 1) < 0) {
            throw Exceptions::Exception("Failed to increment eventfd counter - error number: "
                + std::to_string(errno));
        }
    }

    void EventNotifier::clear() {
        eventfd_t counter = {};
        if (::eventfd_read(this->fileDescriptor.value(), &counter) < 0 && errno != EAGAIN) {
            throw Exceptions::Exception("Failed to clear EventNotifier object - eventfd_read failed - "
                "error number: " + std::to_string(errno));
        }
    }

    void EventNotifier::close() {
        if (this->fileDescriptor.value_or(-1) >= 0) {
            ::close(this->fileDescriptor.value());
        }
    }
}
