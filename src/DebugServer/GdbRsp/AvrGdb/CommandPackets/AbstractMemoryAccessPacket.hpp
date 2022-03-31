#pragma once

#include <cstdint>
#include <optional>

#include "src/DebugServer/GdbRsp/CommandPackets/CommandPacket.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace Bloom::DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The ReadMemory class implements a structure for "m" packets. Upon receiving these packets, the server is
     * expected to read memory from the target and send it the client.
     */
    class AbstractMemoryAccessPacket: public Bloom::DebugServer::Gdb::CommandPackets::CommandPacket
    {
    public:
        explicit AbstractMemoryAccessPacket(const std::vector<unsigned char>& rawPacket): CommandPacket(rawPacket) {};

    protected:
        /**
         * The mask used by the AVR GDB client to encode the memory type into memory addresses.
         */
        static constexpr std::uint32_t AVR_GDB_MEMORY_ADDRESS_MASK = 0xFE0000U;

        /**
         * avr-gdb uses the most significant 15 bits in memory addresses to indicate the type of memory being
         * addressed.
         *
         * @param address
         * @return
         */
        Targets::TargetMemoryType getMemoryTypeFromGdbAddress(std::uint32_t address) {
            if ((address & AbstractMemoryAccessPacket::AVR_GDB_MEMORY_ADDRESS_MASK) != 0U) {
                return Targets::TargetMemoryType::RAM;
            }

            return Targets::TargetMemoryType::FLASH;
        }

        /**
         * Strips the most significant 15 bits from a GDB memory address.
         *
         * @param address
         * @return
         */
        std::uint32_t removeMemoryTypeIndicatorFromGdbAddress(std::uint32_t address) {
            return (address & AbstractMemoryAccessPacket::AVR_GDB_MEMORY_ADDRESS_MASK) != 0U
                ? (address & ~(AbstractMemoryAccessPacket::AVR_GDB_MEMORY_ADDRESS_MASK))
                : address;
        }
    };
}
