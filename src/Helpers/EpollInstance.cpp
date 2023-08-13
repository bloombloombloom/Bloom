#include "EpollInstance.hpp"

#include <string>
#include <cerrno>
#include <unistd.h>
#include <array>

#include "src/Exceptions/Exception.hpp"

using Exceptions::Exception;

EpollInstance::EpollInstance() {
    this->fileDescriptor = ::epoll_create(1);

    if (this->fileDescriptor < 0) {
        throw Exception(
            "Failed to create epoll instance - error number " + std::to_string(errno)
                + " returned."
        );
    }
}

void EpollInstance::addEntry(int fileDescriptor, std::uint16_t eventMask) {
    struct ::epoll_event event = {
        .events = eventMask,
        .data = {
            .fd = fileDescriptor
        }
    };

    if (::epoll_ctl(this->fileDescriptor.value(), EPOLL_CTL_ADD, fileDescriptor, &event) != 0) {
        throw Exception(
            "Failed to add entry to epoll instance - error number " + std::to_string(errno) + " returned."
        );
    }
}

void EpollInstance::removeEntry(int fileDescriptor) {
    if (::epoll_ctl(this->fileDescriptor.value(), EPOLL_CTL_DEL, fileDescriptor, NULL) != 0) {
        throw Exception(
            "Failed to remove entry from epoll instance - error number " + std::to_string(errno)
                + " returned."
        );
    }
}

std::optional<int> EpollInstance::waitForEvent(std::optional<std::chrono::milliseconds> timeout) const {
    std::array<struct epoll_event, 1> events = {};

    const auto eventCount = ::epoll_wait(
        this->fileDescriptor.value(),
        events.data(),
        1,
        timeout.has_value() ? static_cast<int>(timeout->count()) : -1
    );

    if (eventCount < 1) {
        return std::nullopt;
    }

    return static_cast<int>(events.at(0).data.fd);
}

EpollInstance::EpollInstance(EpollInstance&& other) noexcept
    : fileDescriptor(other.fileDescriptor)
{
    other.fileDescriptor = std::nullopt;
}

EpollInstance::~EpollInstance() noexcept {
    if (this->fileDescriptor.value_or(-1) >= 0) {
        ::close(this->fileDescriptor.value());
    }
}
