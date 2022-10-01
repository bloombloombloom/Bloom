#include "ReadMemory.hpp"

#include <bitset>
#include <cmath>

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    ReadMemory::ReadMemory(
        const Avr8MemoryType& type,
        std::uint32_t address,
        std::uint32_t bytes,
        const std::set<std::uint32_t>& excludedAddresses
    )
        : Avr8GenericCommandFrame()
    {
        /*
         * The read memory command consists of 11 + (bytes / 8) bytes:
         * 1. Command ID (0x21 for the general read memory command, 0x22 for reading with a mask)
         * 2. Version (0x00)
         * 3. Memory type (Avr8MemoryType)
         * 4. Start address (4 bytes)
         * 5. Number of bytes to read (4 bytes)
         * 6. Mask to apply (bytes / 8) - only required if we're using the masked read command (command ID 0x22).
         */
        this->payload = std::vector<unsigned char>(11, 0x00);
        this->payload[0] = excludedAddresses.empty() ? 0x21 : 0x22;
        this->payload[1] = 0x00;
        this->payload[2] = static_cast<unsigned char>(type);
        this->payload[3] = static_cast<unsigned char>(address);
        this->payload[4] = static_cast<unsigned char>(address >> 8);
        this->payload[5] = static_cast<unsigned char>(address >> 16);
        this->payload[6] = static_cast<unsigned char>(address >> 24);
        this->payload[7] = static_cast<unsigned char>(bytes);
        this->payload[8] = static_cast<unsigned char>(bytes >> 8);
        this->payload[9] = static_cast<unsigned char>(bytes >> 16);
        this->payload[10] = static_cast<unsigned char>(bytes >> 24);

        if (!excludedAddresses.empty()) {
            const auto endAddress = address + (bytes - 1);

            constexpr auto byteBitSize = std::numeric_limits<unsigned char>::digits;
            auto byteBitset = std::bitset<byteBitSize>();
            byteBitset.reset();

            for (std::uint32_t address = address; address <= endAddress; address++) {
                auto addressIndex = address - address;
                auto bitIndex = static_cast<std::size_t>(
                    addressIndex - (std::floor(addressIndex / byteBitSize) * byteBitSize)
                );

                if (!excludedAddresses.contains(address)) {
                    byteBitset[bitIndex] = true;
                }

                if (address > 0 && (bitIndex == 7 || address == endAddress)) {
                    this->payload.emplace_back(byteBitset.to_ulong());
                    byteBitset.reset();
                }
            }
        }
    }
}
