#include "Connection.hpp"

#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <fcntl.h>
#include <algorithm>

#include "Exceptions/ClientDisconnected.hpp"
#include "Exceptions/DebugServerInterrupted.hpp"
#include "Exceptions/ClientCommunicationError.hpp"

#include "src/Exceptions/Exception.hpp"

#include "src/Logger/Logger.hpp"

namespace Bloom::DebugServer::Gdb
{
    using namespace Exceptions;
    using namespace Bloom::Exceptions;

    using ResponsePackets::ResponsePacket;

    Connection::Connection(int serverSocketFileDescriptor, EventFdNotifier& interruptEventNotifier)
        : interruptEventNotifier(interruptEventNotifier)
    {
        this->accept(serverSocketFileDescriptor);

        ::fcntl(
            this->socketFileDescriptor.value(),
            F_SETFL,
            ::fcntl(this->socketFileDescriptor.value(), F_GETFL, 0) | O_NONBLOCK
        );

        this->epollInstance.addEntry(
            this->socketFileDescriptor.value(),
            static_cast<std::uint16_t>(::EPOLL_EVENTS::EPOLLIN)
        );
        this->enableReadInterrupts();
    }

    Connection::~Connection() {
        this->close();
    }

    std::string Connection::getIpAddress() const {
        std::array<char, INET_ADDRSTRLEN> ipAddress = {};

        if (::inet_ntop(AF_INET, &(socketAddress.sin_addr), ipAddress.data(), INET_ADDRSTRLEN) == nullptr) {
            throw Exception("Failed to convert client IP address to text form.");
        }

        return std::string(ipAddress.data());
    }

    std::vector<RawPacket> Connection::readRawPackets() {
        std::vector<RawPacket> output;

        do {
            const auto bytes = this->read();

            std::size_t bufferSize = bytes.size();
            for (std::size_t byteIndex = 0; byteIndex < bufferSize; byteIndex++) {
                auto byte = bytes[byteIndex];

                if (byte == 0x03) {
                    /*
                     * This is an interrupt packet - it doesn't carry any of the usual packet frame bytes, so we'll
                     * just add them here, in order to keep things consistent.
                     *
                     * Because we're effectively faking the packet frame, we can use any value for the checksum.
                     */
                    output.push_back({'$', byte, '#', 'F', 'F'});
                    continue;
                }

                if (byte == '$') {
                    // Beginning of packet
                    RawPacket rawPacket;
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

                    if (!validPacket) {
                        Logger::warning("GDB client sent invalid packet data - ignoring");
                        continue;
                    }

                    Logger::debug("Read GDB packet: " + std::string(rawPacket.begin(), rawPacket.end()));

                    // Acknowledge receipt
                    this->write({'+'});

                    output.emplace_back(std::move(rawPacket));
                    byteIndex = packetIndex;
                }
            }

        } while (output.empty());

        return output;
    }

    void Connection::writePacket(const ResponsePacket& packet) {
        // Write the packet repeatedly until the GDB client acknowledges it.
        int attempts = 0;
        const auto rawPacket = packet.toRawPacket();

        Logger::debug("Writing GDB packet: " + std::string(rawPacket.begin(), rawPacket.end()));

        do {
            if (attempts > 10) {
                throw ClientCommunicationError(
                    "Failed to write GDB response packet - client failed to acknowledge receipt - retry limit reached"
                );
            }

            this->write(rawPacket);
            attempts++;
        } while (this->readSingleByte(false).value_or(0) != '+');
    }

    void Connection::accept(int serverSocketFileDescriptor) {
        int socketAddressLength = sizeof(this->socketAddress);

        const auto socketFileDescriptor = ::accept(
            serverSocketFileDescriptor,
            reinterpret_cast<sockaddr*>(&(this->socketAddress)),
            reinterpret_cast<socklen_t*>(&socketAddressLength)
        );

        if (socketFileDescriptor < 0) {
            throw Exception("Failed to accept GDB Remote Serial Protocol connection");
        }

        this->socketFileDescriptor = socketFileDescriptor;
    }

    void Connection::close() noexcept {
        if (this->socketFileDescriptor.value_or(-1) >= 0) {
            ::close(this->socketFileDescriptor.value());
            this->socketFileDescriptor = std::nullopt;
        }
    }

    std::vector<unsigned char> Connection::read(
        std::optional<std::size_t> bytes,
        bool interruptible,
        std::optional<std::chrono::milliseconds> timeout
    ) {
        auto output = std::vector<unsigned char>();

        if (this->readInterruptEnabled != interruptible) {
            if (interruptible) {
                this->enableReadInterrupts();

            } else {
                this->disableReadInterrupts();
            }
        }

        // Clear any previous interrupts that are still hanging around
        this->interruptEventNotifier.clear();

        const auto eventFileDescriptor = this->epollInstance.waitForEvent(timeout);

        if (!eventFileDescriptor.has_value()) {
            // Timed out
            return output;
        }

        if (eventFileDescriptor.value() == this->interruptEventNotifier.getFileDescriptor()) {
            // Interrupted
            this->interruptEventNotifier.clear();
            throw DebugServerInterrupted();
        }

        const auto bytesToRead = bytes.value_or(Connection::ABSOLUTE_MAXIMUM_PACKET_READ_SIZE);
        output.resize(bytesToRead, 0x00);

        const auto bytesRead = ::read(
            this->socketFileDescriptor.value(),
            output.data(),
            bytesToRead
        );

        if (bytesRead < 0) {
            throw ClientCommunicationError(
                "Failed to read data from GDB client - error code: " + std::to_string(errno)
            );
        }

        if (bytesRead == 0) {
            // Client has disconnected
            throw ClientDisconnected();
        }

        if (bytesRead != output.size()) {
            output.resize(static_cast<unsigned long>(std::max(ssize_t{0}, bytesRead)));
        }

        return output;
    }

    std::optional<unsigned char> Connection::readSingleByte(bool interruptible) {
        auto bytes = this->read(1, interruptible, std::chrono::milliseconds(300));

        if (!bytes.empty()) {
            return bytes.front();
        }

        return std::nullopt;
    }

    void Connection::write(const std::vector<unsigned char>& buffer) {
        if (::write(this->socketFileDescriptor.value(), buffer.data(), buffer.size()) == -1) {
            if (errno == EPIPE || errno == ECONNRESET) {
                // Connection was closed
                throw ClientDisconnected();
            }

            throw ClientCommunicationError(
                "Failed to write " + std::to_string(buffer.size()) + " bytes to GDP client socket - error no: "
                    + std::to_string(errno)
            );
        }
    }

    void Connection::disableReadInterrupts() {
        this->epollInstance.removeEntry(this->interruptEventNotifier.getFileDescriptor());

        this->readInterruptEnabled = false;
    }

    void Connection::enableReadInterrupts() {
        this->epollInstance.addEntry(
            this->interruptEventNotifier.getFileDescriptor(),
            static_cast<std::uint16_t>(::EPOLL_EVENTS::EPOLLIN)
        );

        this->readInterruptEnabled = true;
    }
}
