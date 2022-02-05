#include "AvrCommand.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr
{
    std::vector<unsigned char> AvrCommand::getData() const {
        std::vector<unsigned char> data;
        auto commandPacket = this->getCommandPacket();
        std::size_t commandPacketSize = commandPacket.size();
        data.resize(3 + commandPacketSize);
        // FragmentInfo byte
        data[0] = static_cast<unsigned char>((this->getFragmentNumber() << 4) | this->getFragmentCount());

        // Size byte
        data[1] = static_cast<unsigned char>(commandPacketSize >> 8);
        data[2] = static_cast<unsigned char>(commandPacketSize & 0xFF);

        if (commandPacketSize > 0) {
            data.insert(data.begin() + 3, commandPacket.begin(), commandPacket.end());
        }

        return data;
    }
}
