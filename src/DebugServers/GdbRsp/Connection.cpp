#include "Connection.hpp"

#include <sys/socket.h>
#include <sys/epoll.h>
#include <cerrno>
#include <fcntl.h>

#include "Exceptions/ClientDisconnected.hpp"
#include "Exceptions/ClientCommunicationError.hpp"
#include "src/Exceptions/Exception.hpp"
#include "src/Exceptions/DebugServerInterrupted.hpp"

#include "src/Logger/Logger.hpp"

namespace Bloom::DebugServers::Gdb
{
    using namespace Exceptions;
    using namespace Bloom::Exceptions;

    using ResponsePackets::ResponsePacket;

    void Connection::accept(int serverSocketFileDescriptor) {
        int socketAddressLength = sizeof(this->socketAddress);

        this->socketFileDescriptor = ::accept(
            serverSocketFileDescriptor,
            (sockaddr*) &(this->socketAddress),
            (socklen_t*) &socketAddressLength
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
            throw Exception(
                "Failed to create event FD for GDB client connection - could not add client connection "
                    "socket FD to epoll FD"
            );
        }

        this->enableReadInterrupts();
    }

    void Connection::close() noexcept {
        if (this->socketFileDescriptor > 0) {
            ::close(this->socketFileDescriptor);
            this->socketFileDescriptor = 0;
        }
    }

    std::vector<RawPacketType> Connection::readRawPackets() {
        std::vector<RawPacketType> output;

        const auto bytes = this->read();

        std::size_t bufferSize = bytes.size();
        for (std::size_t byteIndex = 0; byteIndex < bufferSize; byteIndex++) {
            auto byte = bytes[byteIndex];

            if (byte == 0x03) {
                /*
                 * This is an interrupt packet - it doesn't carry any of the usual packet frame bytes, so we'll just
                 * add them here, in order to keep things consistent.
                 *
                 * Because we're effectively faking the packet frame, we can use any value for the checksum.
                 */
                output.push_back({'$', byte, '#', 'F', 'F'});

            } else if (byte == '$') {
                // Beginning of packet
                RawPacketType rawPacket;
                rawPacket.push_back('$');

                auto packetIndex = byteIndex;
                bool validPacket = false;
                bool isByteEscaped = false;

                for (packetIndex++; packetIndex < bufferSize; packetIndex++) {
                    byte = bytes[packetIndex];

                    if (byte == '}' && !isByteEscaped) {
                        isByteEscaped = true;
                        continue;
                    }

                    if (!isByteEscaped) {
                        if (byte == '$') {
                            // Unexpected end of packet
                            validPacket = false;
                            break;
                        }

                        if (byte == '#') {
                            // End of packet data
                            if ((bufferSize - 1) < (packetIndex + 2)) {
                                // There should be at least two more bytes in the buffer, for the checksum.
                                break;
                            }

                            rawPacket.push_back(byte);

                            // Add the checksum bytes and break the loop
                            rawPacket.push_back(bytes[++packetIndex]);
                            rawPacket.push_back(bytes[++packetIndex]);
                            validPacket = true;
                            break;
                        }

                    } else {
                        // Escaped bytes are XOR'd with a 0x20 mask.
                        byte ^= 0x20;
                        isByteEscaped = false;
                    }

                    rawPacket.push_back(byte);
                }

                if (validPacket) {
                    // Acknowledge receipt
                    this->write({'+'});

                    output.push_back(rawPacket);
                    byteIndex = packetIndex;
                }
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
        } while (this->readSingleByte(false).value_or(0) != '+');
    }

    std::vector<unsigned char> Connection::read(size_t bytes, bool interruptible, std::optional<int> msTimeout) {
        auto output = std::vector<unsigned char>();
        constexpr size_t bufferSize = 1024;
        std::array<unsigned char, bufferSize> buffer = {};
        ssize_t bytesRead = 0;

        if (interruptible) {
            if (this->readInterruptEnabled != interruptible) {
                this->enableReadInterrupts();

            } else {
                // Clear any previous interrupts that are still hanging around
                this->interruptEventNotifier.clear();
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
                auto fileDescriptor = events.at(i).data.fd;

                if (fileDescriptor == this->interruptEventNotifier.getFileDescriptor()) {
                    // Interrupted
                    this->interruptEventNotifier.clear();
                    throw DebugServerInterrupted();
                }
            }

            size_t bytesToRead = (bytes > bufferSize || bytes == 0) ? bufferSize : bytes;
            while (
                bytesToRead > 0
                && (bytesRead = ::read(this->socketFileDescriptor, buffer.data(), bytesToRead)) > 0
            ) {
                output.insert(output.end(), buffer.begin(), buffer.begin() + bytesRead);

                if (bytesRead < bytesToRead) {
                    // No more data available
                    break;
                }

                bytesToRead =
                    ((bytes - output.size()) > bufferSize || bytes == 0) ? bufferSize : (bytes - output.size());
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
            }

            throw ClientCommunicationError("Failed to write " + std::to_string(buffer.size())
                + " bytes to GDP client socket - error no: "
                + std::to_string(errno));
        }
    }

    void Connection::disableReadInterrupts() {
        if (::epoll_ctl(
            this->eventFileDescriptor,
            EPOLL_CTL_DEL,
            this->interruptEventNotifier.getFileDescriptor(),
            NULL) != 0
        ) {
            throw Exception("Failed to disable GDB client connection read interrupts - epoll_ctl failed");
        }

        this->readInterruptEnabled = false;
    }

    void Connection::enableReadInterrupts() {
        auto interruptFileDescriptor = this->interruptEventNotifier.getFileDescriptor();
        struct epoll_event event = {};
        event.events = EPOLLIN;
        event.data.fd = interruptFileDescriptor;

        if (::epoll_ctl(this->eventFileDescriptor, EPOLL_CTL_ADD, interruptFileDescriptor, &event) != 0) {
            throw Exception("Failed to enable GDB client connection read interrupts - epoll_ctl failed");
        }

        this->readInterruptEnabled = true;
    }
}
