#include "EventFdNotifier.hpp"

#include <sys/eventfd.h>
#include <unistd.h>
#include <cerrno>
#include <string>

#include "src/Exceptions/Exception.hpp"

using Exceptions::Exception;

EventFdNotifier::EventFdNotifier() {
    this->fileDescriptor = ::eventfd(0, ::EFD_NONBLOCK);

    if (this->fileDescriptor < 0) {
        throw Exception{"Failed to create eventfd object - error number " + std::to_string(errno) + " returned."};
    }
}

EventFdNotifier::EventFdNotifier(EventFdNotifier&& other) noexcept
    : fileDescriptor(other.fileDescriptor)
{
    other.fileDescriptor = std::nullopt;
}

EventFdNotifier::~EventFdNotifier() noexcept {
    this->close();
}

void EventFdNotifier::notify() {
    if (::eventfd_write(this->fileDescriptor.value(), 1) < 0) {
        throw Exceptions::Exception{"Failed to increment eventfd counter - error number: " + std::to_string(errno)};
    }
}

void EventFdNotifier::clear() {
    auto counter = ::eventfd_t{};
    if (::eventfd_read(this->fileDescriptor.value(), &counter) < 0 && errno != EAGAIN) {
        throw Exceptions::Exception{
            "Failed to clear EventFdNotifier object - eventfd_read failed - " "error number: " + std::to_string(errno)
        };
    }
}

void EventFdNotifier::close() {
    if (this->fileDescriptor.value_or(-1) >= 0) {
        ::close(this->fileDescriptor.value());
    }
}
