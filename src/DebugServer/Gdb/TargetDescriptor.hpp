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
        virtual std::optional<GdbRegisterNumberType> getRegisterNumberFromTargetRegisterDescriptor(
            const Targets::TargetRegisterDescriptor& registerDescriptor
        ) const = 0;

        /**
         * Should retrieve the GDB register descriptor for a given GDB register number.
         *
         * @param number
         * @return
         */
        virtual const RegisterDescriptor& getRegisterDescriptorFromNumber(GdbRegisterNumberType number) const = 0;

        /**
         * Should retrieve the mapped target register descriptor for a given GDB register number.
         *
         * @param number
         * @return
         */
        virtual const Targets::TargetRegisterDescriptor& getTargetRegisterDescriptorFromNumber(
            GdbRegisterNumberType number
        ) const = 0;

        /**
         * Should return all allocated GDB register numbers for the target.
         *
         * @return
         */
        virtual const std::vector<GdbRegisterNumberType>& getRegisterNumbers() const = 0;

    private:
        BiMap<Targets::TargetMemoryType, std::uint32_t> memoryOffsetsByType;
        std::set<std::uint32_t> memoryOffsets;
    };
}
