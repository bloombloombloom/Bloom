#include "ResponsePacket.hpp"

namespace DebugServer::Gdb::ResponsePackets
{
    RawPacket ResponsePacket::toRawPacket() const {
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
        auto checkSum = QStringLiteral("%1").arg(dataSum % 256, 2, 16, QLatin1Char{'0'}).toStdString();

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
}
