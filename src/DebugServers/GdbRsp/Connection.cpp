#include "Connection.hpp"

#include <sys/socket.h>
#include <sys/epoll.h>
#include <cerrno>
#include <fcntl.h>

#include "CommandPackets/CommandPacketFactory.hpp"

#include "Exceptions/ClientDisconnected.hpp"
#include "Exceptions/ClientCommunicationError.hpp"
#include "src/Exceptions/Exception.hpp"
#include "src/Exceptions/DebugServerInterrupted.hpp"

#include "src/Logger/Logger.hpp"

using namespace Bloom::DebugServers::Gdb;
using namespace Bloom::DebugServers::Gdb::CommandPackets;
using namespace Bloom::DebugServers::Gdb::ResponsePackets;
using namespace Bloom::DebugServers::Gdb::Exceptions;
using namespace Bloom::Exceptions;

void Connection::accept(int serverSocketFileDescriptor) {
    int socketAddressLength = sizeof(this->socketAddress);

    this->socketFileDescriptor = ::accept(
        serverSocketFileDescriptor,
        (sockaddr*)& (this->socketAddress),
        (socklen_t*)& socketAddressLength
    );

    if (this->socketFileDescriptor == -1) {
        throw Exception("Failed to accept GDB Remote Serial Protocol connection");
    }

    ::fcntl(
        this->socketFileDescriptor,
        F_SETFL,
        fcntl(this->socketFileDescriptor, F_GETFL, 0) | O_NONBLOCK
    );

    // Create event FD
    this->eventFileDescriptor = ::epoll_create(2);
    struct epoll_event event = {};
    event.events = EPOLLIN;
    event.data.fd = this->socketFileDescriptor;

    if (::epoll_ctl(this->eventFileDescriptor, EPOLL_CTL_ADD, this->socketFileDescriptor, &event) != 0) {
        throw Exception("Failed to create event FD for GDB client connection - could not add client connection "
                        "socket FD to epoll FD");
    }

    this->enableReadInterrupts();
}

void Connection::close() noexcept {
    if (this->socketFileDescriptor > 0) {
        ::close(this->socketFileDescriptor);
        this->socketFileDescriptor = 0;
    }
}

std::vector<std::unique_ptr<CommandPacket>> Connection::readPackets() {
    auto buffer = this->read();
    Logger::debug("GDB client data received (" + std::to_string(buffer.size()) + " bytes): " + std::string(buffer.begin(), buffer.end()));

    auto rawPackets = CommandPacketFactory::extractRawPackets(buffer);
    std::vector<std::unique_ptr<CommandPacket>> output;

    for (const auto& rawPacket : rawPackets) {
        try {
            output.push_back(CommandPacketFactory::create(rawPacket));
            this->write({'+'});

        } catch (const ClientDisconnected& exception) {
            throw exception;

        } catch (const Exception& exception) {
            Logger::error("Failed to parse GDB packet - " + exception.getMessage());
            this->write({'-'});
        }
    }

    return output;
}

void Connection::writePacket(const ResponsePacket& packet) {
    // Write the packet repeatedly until the GDB client acknowledges it.
    int attempts = 0;
    auto rawPacket = packet.toRawPacket();

    do {
        if (attempts > 10) {
            throw ClientCommunicationError("Failed to write GDB response packet - client failed to "
               "acknowledge receipt - retry limit reached");
        }

        this->write(rawPacket);
        attempts++;

    } while(this->readSingleByte(false).value_or(0) != '+');
}

std::vector<unsigned char> Connection::read(size_t bytes, bool interruptible, std::optional<int> msTimeout) {
    auto output = std::vector<unsigned char>();
    constexpr size_t bufferSize = 1024;
    std::array<unsigned char, bufferSize> buffer = {};
    ssize_t bytesRead;

    if (interruptible) {
        if (this->readInterruptEnabled != interruptible) {
            this->enableReadInterrupts();
        } else {
            // Clear any previous interrupts that are still hanging around
            this->interruptEventNotifier->clear();
        }
    }

    if (this->readInterruptEnabled != interruptible && !interruptible) {
        this->disableReadInterrupts();
    }

    std::array<struct epoll_event, 1> events = {};

    int eventCount = ::epoll_wait(
        this->eventFileDescriptor,
        events.data(),
        1,
        msTimeout.value_or(-1)
    );

    if (eventCount > 0) {
        for (size_t i = 0; i < eventCount; i++) {
            auto fileDescriptor = events[i].data.fd;

            if (fileDescriptor == this->interruptEventNotifier->getFileDescriptor()) {
                // Interrupted
                this->interruptEventNotifier->clear();
                throw DebugServerInterrupted();
            }
        }

        size_t bytesToRead = (bytes > bufferSize || bytes == 0) ? bufferSize : bytes;
        while (bytesToRead > 0 && (bytesRead = ::read(this->socketFileDescriptor, buffer.data(), bytesToRead)) > 0) {
            output.insert(output.end(), buffer.begin(), buffer.begin() + bytesRead);

            if (bytesRead < bytesToRead) {
                // No more data available
                break;
            }

            bytesToRead = ((bytes - output.size()) > bufferSize || bytes == 0) ? bufferSize : (bytes - output.size());
        }

        if (output.empty()) {
            // EOF means the client has disconnected
            throw ClientDisconnected();
        }
    }

    return output;
}

std::optional<unsigned char> Connection::readSingleByte(bool interruptible) {
    auto bytes = this->read(1, interruptible, 300);

    if (!bytes.empty()) {
        return bytes.front();
    }

    return std::nullopt;
}

void Connection::write(const std::vector<unsigned char>& buffer) {
    Logger::debug("Writing packet: " + std::string(buffer.begin(), buffer.end()));
    if (::write(this->socketFileDescriptor, buffer.data(), buffer.size()) == -1) {
        if (errno == EPIPE || errno == ECONNRESET) {
            // Connection was closed
            throw ClientDisconnected();

        } else {
            throw ClientCommunicationError("Failed to write " + std::to_string(buffer.size())
            + " bytes to GDP client socket - error no: " + std::to_string(errno));
        }
    }
}

void Connection::disableReadInterrupts() {
    if (::epoll_ctl(
        this->eventFileDescriptor,
        EPOLL_CTL_DEL,
        this->interruptEventNotifier->getFileDescriptor(),
        NULL) != 0
    ) {
        throw Exception("Failed to disable GDB client connection read interrupts - epoll_ctl failed");
    }

    this->readInterruptEnabled = false;
}

void Connection::enableReadInterrupts() {
    auto interruptFileDescriptor = this->interruptEventNotifier->getFileDescriptor();
    struct epoll_event event = {};
    event.events = EPOLLIN;
    event.data.fd = interruptFileDescriptor;

    if (::epoll_ctl(this->eventFileDescriptor, EPOLL_CTL_ADD, interruptFileDescriptor, &event) != 0) {
        throw Exception("Failed to enable GDB client connection read interrupts - epoll_ctl failed");
    }

    this->readInterruptEnabled = true;
}
