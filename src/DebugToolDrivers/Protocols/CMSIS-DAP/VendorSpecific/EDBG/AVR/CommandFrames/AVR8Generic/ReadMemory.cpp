#include "ReadMemory.hpp"

#include <bitset>
#include <cmath>

using namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic;

std::vector<unsigned char> ReadMemory::getPayload() const {
    /*
     * The read memory command consists of 11/11 + (this->bytes / 8) bytes:
     * 1. Command ID (0x21 for the general read memory command, 0x22 for reading with a mask)
     * 2. Version (0x00)
     * 3. Memory type (Avr8MemoryType)
     * 4. Start address (4 bytes)
     * 5. Number of bytes to read (4 bytes)
     * 6. Mask to apply (this->bytes / 8) - only required if we're using the masked read command (command ID 0x22).
     */
    auto output = std::vector<unsigned char>(11, 0x00);
    output[0] = this->excludedAddresses.empty() ? 0x21 : 0x22;
    output[1] = 0x00;
    output[2] = static_cast<unsigned char>(this->type);
    output[3] = static_cast<unsigned char>(this->address);
    output[4] = static_cast<unsigned char>(this->address >> 8);
    output[5] = static_cast<unsigned char>(this->address >> 16);
    output[6] = static_cast<unsigned char>(this->address >> 24);
    output[7] = static_cast<unsigned char>(this->bytes);
    output[8] = static_cast<unsigned char>(this->bytes >> 8);
    output[9] = static_cast<unsigned char>(this->bytes >> 16);
    output[10] = static_cast<unsigned char>(this->bytes >> 24);

    if (!this->excludedAddresses.empty()) {
        const auto endAddress = this->address + (this->bytes - 1);

        constexpr auto byteBitSize = std::numeric_limits<unsigned char>::digits;
        auto byteBitset = std::bitset<byteBitSize>();
        byteBitset.reset();

        for (std::uint32_t address = this->address; address <= endAddress; address++) {
            auto addressIndex = address - this->address;
            auto bitIndex = static_cast<std::size_t>(
                addressIndex - (std::floor(addressIndex / byteBitSize) * byteBitSize)
            );

            if (!this->excludedAddresses.contains(address)) {
                byteBitset[bitIndex] = 1;
            }

            if (address > 0 && (bitIndex == 7 || address == endAddress)) {
                output.emplace_back(byteBitset.to_ulong());
                byteBitset.reset();
            }
        }
    }

    return output;
}
