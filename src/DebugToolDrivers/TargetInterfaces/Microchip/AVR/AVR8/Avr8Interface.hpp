#pragma once

#include <cstdint>
#include <set>

#include "src/Targets/Microchip/AVR/TargetSignature.hpp"
#include "src/Targets/Microchip/AVR/AVR8/TargetParameters.hpp"
#include "src/Targets/TargetState.hpp"
#include "src/Targets/TargetRegister.hpp"
#include "src/Targets/TargetMemory.hpp"
#include "src/ApplicationConfig.hpp"

namespace Bloom::DebugToolDrivers::TargetInterfaces::Microchip::Avr::Avr8
{
    using namespace Bloom;
    using namespace Targets::Microchip::Avr;

    using Targets::TargetState;
    using Targets::TargetRegisterMap;
    using Targets::TargetMemoryBuffer;
    using Targets::TargetMemoryType;
    using Targets::TargetRegister;
    using Targets::TargetRegisters;
    using Targets::Microchip::Avr::TargetSignature;

    /**
     * Interfacing with an AVR8 target can vary significantly, depending on the debug tool being used.
     *
     * This class describes the interface required for interfacing with AVR8 targets.
     *
     * Each debug tool that supports interfacing with AVR8 targets must provide an implementation
     * of this interface class. For example, the Atmel-ICE provides the EdbgAvr8Interface implementation for
     * interfacing with AVR8 targets. See Bloom::DebugToolDrivers::AtmelIce->getAvr8Interface() and
     * Bloom::DebugToolDriver->getAvr8Interface() for more on this.
     */
    class Avr8Interface
    {
    public:
        /**
         * Configures the interface. Any debug tool -> target interface specific configuration should take
         * place here.
         *
         * For example, the EdbgAvr8Interface implementation configures the physical interface and config
         * variant here.
         *
         * @param targetConfig
         */
        virtual void configure(const TargetConfig& targetConfig) = 0;

        /**
         * Should accept Avr8 target parameters for configuration of the interface.
         *
         * @param config
         */
        virtual void setTargetParameters(const Avr8Bit::TargetParameters& config) = 0;

        /**
         * Should initialise the interface between the debug tool and the AVR8 target.
         */
        virtual void init() = 0;

        /**
         * Should stop execution on that target.
         */
        virtual void stop() = 0;

        /**
         * Should resume execution on the AVR8 target.
         */
        virtual void run() = 0;

        /**
         * Continue execution up to a specific byte address.
         */
        virtual void runTo(std::uint32_t address) = 0;

        /**
         * Step execution on teh AVR8 target.
         */
        virtual void step() = 0;

        /**
         * Should reset the AVR8 target.
         */
        virtual void reset() = 0;

        /**
         * Should activate the physical interface between the debug tool and the AVR8 target.
         */
        virtual void activate() = 0;

        /**
         * Should deactivate the physical interface between the debug tool and the AVR8 target.
         */
        virtual void deactivate() = 0;

        /**
         * Should retrieve the AVR8 target signature of the AVR8 target.
         *
         * This method may invoke stop(), as some interfaces are known to require the target to be in a stopped
         * state before the signature can be read.
         *
         * @return
         */
        virtual TargetSignature getDeviceId() = 0;

        /**
         * Should set a software breakpoint at a given address.
         *
         * @param address
         */
        virtual void setBreakpoint(std::uint32_t address) = 0;

        /**
         * Should remove a software breakpoint at a given address.
         *
         * @param address
         */
        virtual void clearBreakpoint(std::uint32_t address) = 0;

        /**
         * Should remove all software and hardware breakpoints on the target.
         */
        virtual void clearAllBreakpoints() = 0;

        /**
         * Should retrieve the current program counter value from the target.
         *
         * @return
         */
        virtual std::uint32_t getProgramCounter() = 0;

        /**
         * Should retrieve the current stack pointer register value from the target.
         *
         * @return
         */
        virtual TargetRegister getStackPointerRegister() = 0;

        /**
         * Should retrieve the current status register value from the target.
         *
         * @return
         */
        virtual TargetRegister getStatusRegister() = 0;

        /**
         * Should update the program counter value on the target.
         *
         * @param programCounter
         */
        virtual void setProgramCounter(std::uint32_t programCounter) = 0;

        /**
         * SHould update the stack pointer register value on the target.
         *
         * @param stackPointerRegister
         */
        virtual void setStackPointerRegister(const TargetRegister& stackPointerRegister) = 0;

        /**
         * Should update the status register value on the target.
         *
         * @param statusRegister
         */
        virtual void setStatusRegister(const TargetRegister& statusRegister) = 0;

        /**
         * Should read the requested general purpose register from the target.
         *
         * @param registerIds
         *  A set of register IDs: 0 -> 31
         *
         * @return
         */
        virtual TargetRegisters readGeneralPurposeRegisters(std::set<size_t> registerIds) = 0;

        /**
         * Should update the value of general purpose registers.
         *
         * @param registers
         */
        virtual void writeGeneralPurposeRegisters(const TargetRegisters& registers) = 0;

        /**
         * Should read memory from the target, for the given memory type.
         *
         * @param memoryType
         * @param startAddress
         * @param bytes
         * @return
         */
        virtual TargetMemoryBuffer readMemory(TargetMemoryType memoryType, std::uint32_t startAddress, std::uint32_t bytes) = 0;

        /**
         * Should write memory to the target, for a given memory type.
         *
         * @param memoryType
         * @param startAddress
         * @param buffer
         */
        virtual void writeMemory(TargetMemoryType memoryType, std::uint32_t startAddress, const TargetMemoryBuffer& buffer) = 0;

        /**
         * Should obtain the current target state.
         *
         * @return
         */
        virtual TargetState getTargetState() = 0;
    };
}