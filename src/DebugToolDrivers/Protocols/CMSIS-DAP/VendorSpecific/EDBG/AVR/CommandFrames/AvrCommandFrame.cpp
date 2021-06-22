#include <math.h>

#include "AvrCommandFrame.hpp"

using namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr;

std::vector<AvrCommand> AvrCommandFrame::generateAvrCommands(std::size_t maximumCommandPacketSize) const {
    auto rawCommandFrame = static_cast<std::vector<unsigned char>>(*this);
    std::size_t commandFrameSize = rawCommandFrame.size();
    auto commandsRequired = static_cast<std::size_t>(
        ceil(static_cast<float>(commandFrameSize) / static_cast<float>(maximumCommandPacketSize))
    );

    std::vector<AvrCommand> avrCommands;
    std::size_t copiedPacketSize = 0;
    for (std::size_t i = 0; i < commandsRequired; i++) {
        AvrCommand avrCommand;
        avrCommand.setFragmentCount(commandsRequired);
        avrCommand.setFragmentNumber(i + 1);
        auto commandPacket = avrCommand.getCommandPacket();

        // If we're on the last packet, the packet size will be what ever is left of the AvrCommandFrame
        std::size_t commandPacketSize = ((i + 1) != commandsRequired) ? maximumCommandPacketSize
            : (commandFrameSize - (maximumCommandPacketSize * i));

        commandPacket.insert(
            commandPacket.end(),
            rawCommandFrame.begin() + static_cast<long>(copiedPacketSize),
            rawCommandFrame.begin() + static_cast<long>(copiedPacketSize + commandPacketSize)
        );

        avrCommand.setCommandPacket(commandPacket);
        avrCommands.push_back(avrCommand);
        copiedPacketSize += commandPacketSize;
    }

    return avrCommands;
}

AvrCommandFrame::operator std::vector<unsigned char> () const {
    auto data = this->getPayload();
    auto dataSize = data.size();

    auto rawCommand = std::vector<unsigned char>(5);

    rawCommand[0] = this->SOF;
    rawCommand[1] = this->getProtocolVersion();

    rawCommand[2] = static_cast<unsigned char>(this->getSequenceId());
    rawCommand[3] = static_cast<unsigned char>(this->getSequenceId() >> 8);

    rawCommand[4] = static_cast<unsigned char>(this->getProtocolHandlerId());

    if (dataSize > 0) {
        rawCommand.insert(
            rawCommand.end(),
            data.begin(),
            data.end()
        );
    }

    return rawCommand;
}
