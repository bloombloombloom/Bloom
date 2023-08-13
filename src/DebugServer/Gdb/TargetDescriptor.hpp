#pragma once

#include <cstdint>
#include <optional>
#include <vector>
#include <set>
#include <map>

#include "src/Helpers/BiMap.hpp"
#include "src/Targets/TargetDescriptor.hpp"
#include "src/Targets/TargetRegister.hpp"
#include "src/Targets/TargetMemory.hpp"

#include "RegisterDescriptor.hpp"

namespace DebugServer::Gdb
{
    /**
     * GDB target descriptor.
     */
    class TargetDescriptor
    {
    public:
        Targets::TargetDescriptor targetDescriptor;
        std::map<GdbRegisterId, RegisterDescriptor> gdbRegisterDescriptorsById;

        explicit TargetDescriptor(
            const Targets::TargetDescriptor& targetDescriptor,
            const BiMap<Targets::TargetMemoryType, std::uint32_t>& memoryOffsetsByType,
            std::map<GdbRegisterId, RegisterDescriptor> gdbRegisterDescriptorsById,
            std::map<Targets::TargetRegisterDescriptorId, GdbRegisterId> gdbRegisterIdsByTargetRegisterDescriptorId,
            std::map<GdbRegisterId, Targets::TargetRegisterDescriptorId> targetRegisterDescriptorIdsByGdbRegisterId
        );

        virtual ~TargetDescriptor() = default;

        std::uint32_t getMemoryOffset(Targets::TargetMemoryType memoryType) const;

        /**
         * Helper method to extract the target memory type (Flash, RAM, etc) from a GDB memory address.
         *
         * @param address
         * @return
         */
        Targets::TargetMemoryType getMemoryTypeFromGdbAddress(std::uint32_t address) const;

        /**
         * Should retrieve the GDB register ID, given a target register descriptor ID. Or std::nullopt if the
         * target register descriptor ID isn't mapped to any GDB register.
         *
         * @param registerDescriptorId
         * @return
         */
        std::optional<GdbRegisterId> getGdbRegisterIdFromTargetRegisterDescriptorId(
            Targets::TargetRegisterDescriptorId targetRegisterDescriptorId
        ) const;

        /**
         * Should retrieve the mapped target register descriptor ID for a given GDB register ID.
         *
         * This function may return std::nullopt if the GDB register ID maps to something that isn't considered a
         * register on our end. For example, for AVR targets, the GDB register ID 34 maps to the program counter. But
         * the program counter is not treated like any other register in Bloom (there's no TargetRegisterDescriptor for
         * it). So in that case, the GDB register ID is not mapped to any target register descriptor ID.
         *
         * @param gdbRegisterId
         * @return
         */
        std::optional<Targets::TargetRegisterDescriptorId> getTargetRegisterDescriptorIdFromGdbRegisterId(
            GdbRegisterId gdbRegisterId
        ) const;

    protected:
        /**
         * When GDB sends us a memory address, the memory type (Flash, RAM, EEPROM, etc) is embedded within. This is
         * done by ORing the address with some constant. For example, for AVR targets, RAM addresses are ORed with
         * 0x00800000. Flash addresses are left unchanged. EEPROM addressing is not supported in GDB (for AVR targets).
         *
         * memoryOffsetsByType is a mapping of memory types to these known constants (which we're calling offsets).
         * Because these offsets vary by target, the mapping lives here, in the GDB target descriptor.
         */
        BiMap<Targets::TargetMemoryType, std::uint32_t> memoryOffsetsByType;

        /**
         * Sorted set of the known memory offsets (see memoryOffsetsByType).
         */
        std::set<std::uint32_t> memoryOffsets;

        std::map<Targets::TargetRegisterDescriptorId, GdbRegisterId> gdbRegisterIdsByTargetRegisterDescriptorId;
        std::map<GdbRegisterId, Targets::TargetRegisterDescriptorId> targetRegisterDescriptorIdsByGdbRegisterId;
    };
}
