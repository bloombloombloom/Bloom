#pragma once

#include <cstdint>
#include <utility>
#include <vector>
#include <netinet/in.h>
#include <queue>
#include <array>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "src/Helpers/EventNotifier.hpp"
#include "src/DebugServers/GdbRsp/CommandPackets/CommandPacket.hpp"
#include "src/DebugServers/GdbRsp/ResponsePackets/ResponsePacket.hpp"

namespace Bloom::DebugServers::Gdb
{
    /**
     * The Connection class represents an active connection between the GDB RSP server and client.
     *
     * All interfacing with the GDB client should take place here.
     */
    class Connection
    {
    public:
        explicit Connection(std::shared_ptr<EventNotifier> interruptEventNotifier)
            : interruptEventNotifier(std::move(interruptEventNotifier))
        {};

        /**
         * Accepts a connection on serverSocketFileDescriptor.
         *
         * @param serverSocketFileDescriptor
         */
        void accept(int serverSocketFileDescriptor);

        /**
         * Closes the connection with the client.
         */
        void close() noexcept;

        /**
         * Obtains the human readable IP address of the connected client.
         *
         * @return
         */
        std::string getIpAddress() {
            std::array<char, INET_ADDRSTRLEN> ipAddress = {};

            if (::inet_ntop(AF_INET, &(socketAddress.sin_addr), ipAddress.data(), INET_ADDRSTRLEN) == nullptr) {
                throw Exceptions::Exception("Failed to convert client IP address to text form.");
            }

            return std::string(ipAddress.data());
        };

        /**
         * Waits for incoming data from the client and returns any received command packets.
         *
         * @return
         */
        std::vector<std::unique_ptr<CommandPackets::CommandPacket>> readPackets();

        /**
         * Sends a response packet to the client.
         *
         * @param packet
         */
        void writePacket(const ResponsePackets::ResponsePacket& packet);

        [[nodiscard]] int getMaxPacketSize() const {
            return this->maxPacketSize;
        }

    private:
        int socketFileDescriptor = -1;
        int eventFileDescriptor = -1;

        struct sockaddr_in socketAddress = {};
        int maxPacketSize = 1024;

        /**
         * The interruptEventNotifier allows us to interrupt blocking IO calls on the GDB debug server.
         * Under the hood, this is just a wrapper for a Linux event notifier. See the EventNotifier class for more.
         */
        std::shared_ptr<EventNotifier> interruptEventNotifier = nullptr;
        bool readInterruptEnabled = false;

        /**
         * Reads data from the client into a raw buffer.
         *
         * @param bytes
         *  Number of bytes to read.
         *
         * @param interruptible
         *  If this flag is set to false, no other component within Bloom will be able to gracefully interrupt
         *  the read (via means of this->interruptEventNotifier). This flag has no effect if this->readInterruptEnabled
         *  is false.
         *
         * @param msTimeout
         *  The timeout in milliseconds. If not supplied, no timeout will be applied.
         *
         * @return
         */
        std::vector<unsigned char> read(std::size_t bytes = 0, bool interruptible = true, std::optional<int> msTimeout = std::nullopt);

        /**
         * Does the same as Connection::read(), but only reads a single byte.
         *
         * @param interruptible
         *  See Connection::read().
         *
         * @return
         */
        std::optional<unsigned char> readSingleByte(bool interruptible = true);

        /**
         * Writes data from a raw buffer to the client connection.
         *
         * @param buffer
         */
        void write(const std::vector<unsigned char>& buffer);

        void disableReadInterrupts();

        void enableReadInterrupts();
    };
}
