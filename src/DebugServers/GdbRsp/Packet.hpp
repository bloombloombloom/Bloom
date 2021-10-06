#pragma once

#include <vector>
#include <memory>
#include <numeric>
#include <QString>
#include <sstream>
#include <iomanip>

namespace Bloom::DebugServers::Gdb
{
    /**
     * The Packet class implements the data structure for GDB RSP packets.
     *
     * Fore more information on the packet data structure, see https://sourceware.org/gdb/onlinedocs/gdb/Overview.html#Overview
     */
    class Packet
    {
    public:
        Packet() = default;
        explicit Packet(const std::vector<unsigned char>& rawPacket) {
            this->init(rawPacket);
        }
        virtual ~Packet() = default;

        [[nodiscard]] virtual std::vector<unsigned char> getData() const {
            return this->data;
        }

        void setData(const std::vector<unsigned char>& data) {
            this->data = data;
        }

        /**
         * Generates a raw packet.
         *
         * @return
         */
        [[nodiscard]] std::vector<unsigned char> toRawPacket() const {
            std::vector<unsigned char> packet = {'$'};
            auto data = this->getData();

            for (const auto& byte : data) {
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
         * Converts raw data to hexadecimal form, the form in which responses are expected to be delivered from the
         * server.
         *
         * @param data
         * @return
         */
        static std::string dataToHex(const std::vector<unsigned char>& data) {
            std::stringstream stream;
            stream << std::hex << std::setfill('0');

            for (const auto& byte : data) {
                stream << std::setw(2) << static_cast<unsigned int>(byte);
            }

            return stream.str();
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

        void init(const std::vector<unsigned char>& rawPacket) {
            this->data.insert(
                this->data.begin(),
                rawPacket.begin() + 1,
                rawPacket.end() - 3
            );
        }
    };
}
