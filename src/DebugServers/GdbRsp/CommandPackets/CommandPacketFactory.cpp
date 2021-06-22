#include <vector>
#include <memory>
#include <map>

#include "CommandPacketFactory.hpp"

using namespace Bloom::DebugServers::Gdb;
using namespace Bloom::DebugServers::Gdb::CommandPackets;

std::unique_ptr<CommandPacket> CommandPacketFactory::create(std::vector<unsigned char> rawPacket) {
    if (rawPacket.size() == 5 && rawPacket[1] == 0x03) {
        // This is an interrupt request - create a fake packet for it
        return std::make_unique<CommandPackets::InterruptExecution>(rawPacket);
    }

    auto rawPacketString = std::string(rawPacket.begin(), rawPacket.end());

    if (rawPacketString.size() >= 2) {
        /*
         * First byte of the raw packet will be 0x24 ('$'), so find() should return 1, not 0, when
         * looking for a command identifier string.
         */
        if (rawPacketString.find("qSupported") == 1) {
            return std::make_unique<CommandPackets::SupportedFeaturesQuery>(rawPacket);

        } else if (rawPacketString[1] == 'g' || rawPacketString[1] == 'p') {
            return std::make_unique<CommandPackets::ReadGeneralRegisters>(rawPacket);

        } else if (rawPacketString[1] == 'P') {
            return std::make_unique<CommandPackets::WriteGeneralRegister>(rawPacket);

        } else if (rawPacketString[1] == 'c') {
            return std::make_unique<CommandPackets::ContinueExecution>(rawPacket);

        } else if (rawPacketString[1] == 's') {
            return std::make_unique<CommandPackets::StepExecution>(rawPacket);

        } else if (rawPacketString[1] == 'm') {
            return std::make_unique<CommandPackets::ReadMemory>(rawPacket);

        }  else if (rawPacketString[1] == 'M') {
            return std::make_unique<CommandPackets::WriteMemory>(rawPacket);

        } else if (rawPacketString[1] == 'Z') {
            return std::make_unique<CommandPackets::SetBreakpoint>(rawPacket);

        } else if (rawPacketString[1] == 'z') {
            return std::make_unique<CommandPackets::RemoveBreakpoint>(rawPacket);
        }
    }

    return std::make_unique<CommandPacket>(rawPacket);
}

std::vector<std::vector<unsigned char>> CommandPacketFactory::extractRawPackets(std::vector<unsigned char> buffer) {
    std::vector<std::vector<unsigned char>> output;

    std::size_t bufferIndex;
    std::size_t bufferSize = buffer.size();
    unsigned char byte;
    for (bufferIndex = 0; bufferIndex < bufferSize; bufferIndex++) {
        byte = buffer[bufferIndex];

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
            std::vector<unsigned char> rawPacket;
            rawPacket.push_back('$');

            auto packetIndex = bufferIndex;
            bool validPacket = false;
            bool isByteEscaped = false;

            for (packetIndex++; packetIndex < bufferSize; packetIndex++) {
                byte = buffer[packetIndex];

                if (byte == '}' && !isByteEscaped) {
                    isByteEscaped = true;
                    continue;
                }

                if (byte == '$' && !isByteEscaped) {
                    // Unexpected end of packet
                    validPacket = false;
                    break;
                }

                if (byte == '#' && !isByteEscaped) {
                    // End of packet data
                    if ((bufferSize - 1) < (packetIndex + 2)) {
                        // There should be at least two more bytes in the buffer, for the checksum.
                        break;
                    }

                    rawPacket.push_back(byte);

                    // Add the checksum bytes and break the loop
                    rawPacket.push_back(buffer[++packetIndex]);
                    rawPacket.push_back(buffer[++packetIndex]);
                    validPacket = true;
                    break;
                }

                if (isByteEscaped) {
                    // Escaped bytes are XOR'd with a 0x20 mask.
                    byte ^= 0x20;
                    isByteEscaped = false;
                }

                rawPacket.push_back(byte);
            }

            if (validPacket) {
                output.push_back(rawPacket);
                bufferIndex = packetIndex;
            }
        }
    }

    return output;
}
