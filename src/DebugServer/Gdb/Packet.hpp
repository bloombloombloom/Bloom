#pragma once

#include <vector>
#include <memory>
#include <numeric>
#include <QString>
#include <sstream>
#include <iomanip>

namespace DebugServer::Gdb
{
    using RawPacket = std::vector<unsigned char>;

    /**
     * The Packet class implements the data structure for GDB RSP packets.
     *
     * Fore more information on the packet data structure, see https://sourceware.org/gdb/onlinedocs/gdb/Overview.html#Overview
     */
    class Packet
    {
    public:
        explicit Packet(const RawPacket& rawPacket) {
            this->init(rawPacket);
        }

        Packet() = default;
        virtual ~Packet() = default;

        Packet(const Packet& other) = default;
        Packet(Packet&& other) = default;

        Packet& operator = (const Packet& other) = default;
        Packet& operator = (Packet&& other) = default;

    protected:
        std::vector<unsigned char> data;

        void init(const RawPacket& rawPacket) {
            this->data.insert(
                this->data.begin(),
                rawPacket.begin() + 1,
                rawPacket.end() - 3
            );
        }
    };
}
