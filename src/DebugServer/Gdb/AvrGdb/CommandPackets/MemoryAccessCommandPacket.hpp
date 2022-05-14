#pragma once

#include <cstdint>
#include <optional>

#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace Bloom::DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The MemoryAccessCommandPacket class is a base class for memory access GDB commands that are specific to the AVR
     * architecture.
     *
     * With the GDB implementation for the AVR architecture, read/write memory commands include a special memory
     * address. The memory type (FLASH, RAM, EEPROM, etc) is embedded within the 7 most significant bits of the second
     * most significant byte of the memory address.
     *
     * This class provides functions to extract and remove the memory type from a given memory address.
     */
    class MemoryAccessCommandPacket: public Bloom::DebugServer::Gdb::CommandPackets::CommandPacket
    {
    public:
        explicit MemoryAccessCommandPacket(const RawPacketType& rawPacket)
            : CommandPacket(rawPacket)
        {};

    protected:
        /**
         * The mask used by the AVR GDB client to encode the memory type into memory addresses.
         */
        static constexpr std::uint32_t AVR_GDB_MEMORY_ADDRESS_MASK = 0x00FE0000U;

        /**
         * avr-gdb uses the most significant 15 bits in memory addresses to indicate the type of memory being
         * addressed.
         *
         * @param address
         * @return
         */
        Targets::TargetMemoryType getMemoryTypeFromGdbAddress(std::uint32_t address) {
            if ((address & MemoryAccessCommandPacket::AVR_GDB_MEMORY_ADDRESS_MASK) != 0U) {
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
            return (address & MemoryAccessCommandPacket::AVR_GDB_MEMORY_ADDRESS_MASK) != 0U
                ? (address & ~(MemoryAccessCommandPacket::AVR_GDB_MEMORY_ADDRESS_MASK))
                : address;
        }
    };
}
