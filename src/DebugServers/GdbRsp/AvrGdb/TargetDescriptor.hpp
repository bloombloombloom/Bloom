#pragma once

#include "src/DebugServers/GdbRsp/TargetDescriptor.hpp"

#include "src/Helpers/BiMap.hpp"

namespace Bloom::DebugServers::Gdb::AvrGdb
{
    class TargetDescriptor: public DebugServers::Gdb::TargetDescriptor
    {
    public:
        BiMap<GdbRegisterNumberType, RegisterDescriptor> registerDescriptorsByGdbNumber = {};
        BiMap<GdbRegisterNumberType, Targets::TargetRegisterDescriptor> targetRegisterDescriptorsByGdbNumber = {};

        explicit TargetDescriptor(const Targets::TargetDescriptor& targetDescriptor);

        /**
         * Should retrieve the GDB register number, given a target register descriptor. Or std::nullopt if the target
         * register descriptor isn't mapped to any GDB register.
         *
         * @param registerDescriptor
         * @return
         */
        std::optional<GdbRegisterNumberType> getRegisterNumberFromTargetRegisterDescriptor(
            const Targets::TargetRegisterDescriptor& registerDescriptor
        ) const override;

        /**
         * Should retrieve the GDB register descriptor for a given GDB register number.
         *
         * @param number
         * @return
         */
        const RegisterDescriptor& getRegisterDescriptorFromNumber(GdbRegisterNumberType number) const override;

        /**
         * Should retrieve the mapped target register descriptor for a given GDB register number.
         *
         * @param number
         * @return
         */
        const Targets::TargetRegisterDescriptor& getTargetRegisterDescriptorFromNumber(
            GdbRegisterNumberType number
        ) const override;

        const std::vector<GdbRegisterNumberType>& getRegisterNumbers() const override;

    private:
        std::vector<GdbRegisterNumberType> registerNumbers = std::vector<GdbRegisterNumberType>(35);

        /**
         * For AVR targets, avr-gdb defines 35 registers in total:
         *
         * Register number 0 through 31 are general purpose registers
         * Register number 32 is the status register (SREG)
         * Register number 33 is the stack pointer register
         * Register number 34 is the program counter register
         *
         * This function will prepare the appropriate GDB register numbers and mappings.
         */
        void loadRegisterMappings();
    };
}
