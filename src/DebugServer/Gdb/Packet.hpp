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

        /**
         * Generates a raw packet.
         *
         * @return
         */
        [[nodiscard]] RawPacket toRawPacket() const {
            std::vector<unsigned char> packet = {'$'};

            for (const auto& byte : this->data) {
                // Escape $ and # characters
                switch (byte) {
                    case '$':
                    case '#': {
                        packet.push_back('}');
                        packet.push_back(byte ^ 0x20);
                    }
                    default: {
                        packet.push_back(byte);
                    }
                }
            }

            auto dataSum = std::accumulate(packet.begin() + 1, packet.end(), 0);
            packet.push_back('#');
            auto checkSum = QStringLiteral("%1").arg(dataSum % 256, 2, 16, QLatin1Char('0')).toStdString();

            if (checkSum.size() < 2) {
                packet.push_back('0');

                if (checkSum.size() < 1) {
                    packet.push_back('0');
                } else {
                    packet.push_back(static_cast<unsigned char>(checkSum[0]));
                }
            }

            packet.push_back(static_cast<unsigned char>(checkSum[0]));
            packet.push_back(static_cast<unsigned char>(checkSum[1]));

            return packet;
        }

        /**
         * Converts data in hexadecimal form to raw data.
         *
         * @param hexData
         * @return
         */
        static std::vector<unsigned char> hexToData(const std::string& hexData) {
            std::vector<unsigned char> output;

            for (auto i = 0; i < hexData.size(); i += 2) {
                auto hexByte = std::string((hexData.begin() + i), (hexData.begin() + i + 2));
                output.push_back(static_cast<unsigned char>(std::stoi(hexByte, nullptr, 16)));
            }

            return output;
        }

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
