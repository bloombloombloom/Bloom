#pragma once

#include <cstdint>
#include <optional>
#include <vector>
#include <set>

#include "src/Helpers/BiMap.hpp"
#include "src/Targets/TargetDescriptor.hpp"
#include "src/Targets/TargetRegister.hpp"
#include "src/Targets/TargetMemory.hpp"

#include "RegisterDescriptor.hpp"

namespace Bloom::DebugServer::Gdb
{
    /**
     * GDB target descriptor.
     */
    class TargetDescriptor
    {
    public:
        Targets::TargetDescriptor targetDescriptor;

        explicit TargetDescriptor(
            const Targets::TargetDescriptor& targetDescriptor,
            const BiMap<Targets::TargetMemoryType, std::uint32_t>& memoryOffsetsByType
        )
            : targetDescriptor(targetDescriptor)
            , memoryOffsetsByType(memoryOffsetsByType)
            , memoryOffsets(memoryOffsetsByType.getValues())
        {}

        virtual ~TargetDescriptor() = default;

        virtual std::uint32_t getMemoryOffset(Targets::TargetMemoryType memoryType) const {
            return this->memoryOffsetsByType.valueAt(memoryType).value_or(0);
        }

        /**
         * Helper method to extract the target memory type (Flash, RAM, etc) from a GDB memory address.
         *
         * @param address
         * @return
         */
        Targets::TargetMemoryType getMemoryTypeFromGdbAddress(std::uint32_t address) const {
            // Start with the largest offset until we find a match
            for (
                auto memoryOffsetIt = this->memoryOffsets.rbegin();
                memoryOffsetIt != this->memoryOffsets.rend();
                ++memoryOffsetIt
            ) {
                if ((address & *memoryOffsetIt) != 0U) {
                    return this->memoryOffsetsByType.at(*memoryOffsetIt);
                }
            }

            return Targets::TargetMemoryType::FLASH;
        }

        /**
         * Should retrieve the GDB register number, given a target register descriptor. Or std::nullopt if the target
         * register descriptor isn't mapped to any GDB register.
         *
         * @param registerDescriptor
         * @return
         */
        virtual std::optional<GdbRegisterNumber> getRegisterNumberFromTargetRegisterDescriptor(
            const Targets::TargetRegisterDescriptor& registerDescriptor
        ) const = 0;

        /**
         * Should retrieve the GDB register descriptor for a given GDB register number.
         *
         * @param number
         * @return
         */
        virtual const RegisterDescriptor& getRegisterDescriptorFromNumber(GdbRegisterNumber number) const = 0;

        /**
         * Should retrieve the mapped target register descriptor for a given GDB register number.
         *
         * @param number
         * @return
         */
        virtual const Targets::TargetRegisterDescriptor& getTargetRegisterDescriptorFromNumber(
            GdbRegisterNumber number
        ) const = 0;

        /**
         * Should return all allocated GDB register numbers for the target.
         *
         * @return
         */
        virtual const std::vector<GdbRegisterNumber>& getRegisterNumbers() const = 0;

    private:
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
    };
}
