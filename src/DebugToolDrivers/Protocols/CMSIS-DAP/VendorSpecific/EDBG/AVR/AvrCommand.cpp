#include "AvrCommand.hpp"

namespace DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr
{
    AvrCommand::AvrCommand(
        std::size_t fragmentCount,
        std::size_t fragmentNumber,
        const std::vector<unsigned char>& commandPacket
    )
        : Command(0x80)
    {
        const auto commandPacketSize = commandPacket.size();
        this->data.reserve(commandPacketSize + 3);

        // FragmentInfo byte
        this->data.emplace_back(static_cast<unsigned char>((fragmentNumber << 4) | fragmentCount));

        // Size byte
        this->data.emplace_back(static_cast<unsigned char>(commandPacketSize >> 8));
        this->data.emplace_back(static_cast<unsigned char>(commandPacketSize & 0xFF));

        if (commandPacketSize > 0) {
            // Packet data
            this->data.insert(this->data.begin() + 3, commandPacket.begin(), commandPacket.end());
        }
    }
}
