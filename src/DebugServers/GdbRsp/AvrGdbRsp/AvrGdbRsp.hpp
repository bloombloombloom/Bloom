#pragma once

#include <cstdint>

#include "src/DebugServers/GdbRsp/GdbRspDebugServer.hpp"
#include "src/DebugServers/GdbRsp/RegisterDescriptor.hpp"
#include "src/Helpers/BiMap.hpp"

namespace Bloom::DebugServers::Gdb
{
    /**
     * The AVR GDB client (avr-gdb) defines a set of parameters relating to AVR targets. These parameters are
     * hardcoded in the AVR GDB source code. The client expects all compatible GDB RSP servers to be aware of
     * these parameters.
     *
     * An example of these hardcoded parameters is target registers and the order in which they are supplied; AVR GDB
     * clients expect 35 registers to be accessible via the server. 32 of these registers are general purpose CPU
     * registers. The GP registers are expected to be followed by the status register (SREG), stack pointer
     * register (SPH & SPL) and the program counter. These must all be given in a specific order, which is
     * pre-determined by the AVR GDB client. See AvrGdbRsp::getRegisterNumberToDescriptorMapping() for more.
     *
     * For more on this, see the AVR GDB source code at https://github.com/bminor/binutils-gdb/blob/master/gdb/avr-tdep.c
     *
     * The AvrGdpRsp class extends the generic GDB RSP debug server and implements these AVR specific parameters.
     */
    class AvrGdbRsp: public GdbRspDebugServer
    {
    public:
        explicit AvrGdbRsp(EventManager& eventManager): GdbRspDebugServer(eventManager) {};

        std::string getName() const override {
            return "AVR GDB Remote Serial Protocol Debug Server";
        }

    protected:
        void init() override;

        void loadRegisterMappings();

        /**
         * avr-gdb uses the most significant 15 bits in memory addresses to indicate the type of memory being
         * addressed.
         *
         * @param address
         * @return
         */
        Targets::TargetMemoryType getMemoryTypeFromGdbAddress(std::uint32_t address) override {
            if (address & this->gdbInternalMemoryMask) {
                return Targets::TargetMemoryType::RAM;
            }

            return Targets::TargetMemoryType::FLASH;
        };

        /**
         * Strips the most significant 15 bits from a GDB memory address.
         *
         * @param address
         * @return
         */
        std::uint32_t removeMemoryTypeIndicatorFromGdbAddress(std::uint32_t address) override {
            return address & this->gdbInternalMemoryMask ? (address & ~(this->gdbInternalMemoryMask)) : address;
        };

        const BiMap<GdbRegisterNumberType, RegisterDescriptor>& getRegisterNumberToDescriptorMapping() override {
            return this->registerDescriptorsByGdbNumber;
        };

        std::optional<GdbRegisterNumberType> getRegisterNumberFromTargetRegisterDescriptor(
            const Targets::TargetRegisterDescriptor& registerDescriptor
        ) override;

        const RegisterDescriptor& getRegisterDescriptorFromNumber(GdbRegisterNumberType number) override;

        const Targets::TargetRegisterDescriptor& getTargetRegisterDescriptorFromNumber(
            GdbRegisterNumberType number
        ) override;

    private:
        /*
         * For AVR targets, avr-gdb defines 35 registers in total:
         *
         * Register number 0 through 31 are general purpose registers
         * Register number 32 is the status register (SREG)
         * Register number 33 is the stack pointer register
         * Register number 34 is the program counter register
         *
         * In this class, we maintain two bidirectional mappings:
         *
         *  - registerDescriptorsByGdbNumber
         *      A mapping of GDB register numbers to GDB register descriptors.
         *
         *  - targetRegisterDescriptorsByGdbNumber
         *      A mapping of GDB register numbers to target register descriptors.
         *
         * The functions above provide an interface for the retrieval of GDB register descriptors and target register
         * descriptors, by their respective GDB register number.
         */
        BiMap<GdbRegisterNumberType, RegisterDescriptor> registerDescriptorsByGdbNumber = {};
        BiMap<GdbRegisterNumberType, Targets::TargetRegisterDescriptor> targetRegisterDescriptorsByGdbNumber = {};

        /**
         * The mask used by the AVR GDB client to encode the memory type into memory addresses.
         * See AvrGdbRsp::getMemoryTypeFromGdbAddress() for more.
         */
        unsigned int gdbInternalMemoryMask = 0xFE0000U;
    };
}
