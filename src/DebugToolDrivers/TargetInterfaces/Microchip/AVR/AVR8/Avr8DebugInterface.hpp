#pragma once

#include <cstdint>
#include <set>
#include <optional>

#include "src/Targets/Microchip/AVR/AVR8/Avr8TargetConfig.hpp"

#include "src/Targets/Microchip/AVR/TargetSignature.hpp"
#include "src/Targets/Microchip/AVR/AVR8/Family.hpp"
#include "src/Targets/Microchip/AVR/AVR8/PhysicalInterface.hpp"
#include "src/Targets/Microchip/AVR/AVR8/ProgramMemorySection.hpp"
#include "src/Targets/Microchip/AVR/AVR8/TargetParameters.hpp"

#include "src/Targets/TargetState.hpp"
#include "src/Targets/TargetRegister.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Bloom::DebugToolDrivers::TargetInterfaces::Microchip::Avr::Avr8
{
    /**
     * Interfacing with an AVR8 target for debugging operations can vary significantly, depending on the debug tool
     * being used. Some debug tools employ different protocols.
     *
     * This class describes the interface required for interfacing with AVR8 targets, for debugging operations.
     *
     * Each debug tool that supports interfacing with AVR8 targets must provide an implementation
     * of this interface class. For example, the Atmel-ICE provides the EdbgAvr8Interface implementation for
     * interfacing with AVR8 targets. See Bloom::DebugToolDrivers::AtmelIce::getAvr8DebugInterface() and
     * Bloom::DebugTool::getAvr8DebugInterface() for more on this.
     */
    class Avr8DebugInterface
    {
    public:
        Avr8DebugInterface() = default;
        virtual ~Avr8DebugInterface() = default;

        Avr8DebugInterface(const Avr8DebugInterface& other) = default;
        Avr8DebugInterface(Avr8DebugInterface&& other) = default;

        Avr8DebugInterface& operator = (const Avr8DebugInterface& other) = default;
        Avr8DebugInterface& operator = (Avr8DebugInterface&& other) = default;

        /**
         * Configures the interface. Any debug tool -> target interface specific configuration should take
         * place here.
         *
         * @param targetConfig
         */
        virtual void configure(const Targets::Microchip::Avr::Avr8Bit::Avr8TargetConfig& targetConfig) = 0;

        /**
         * Sets the target family, independent of other configuration.
         *
         * @param family
         */
        virtual void setFamily(Targets::Microchip::Avr::Avr8Bit::Family family) = 0;

        /**
         * Should accept Avr8 target parameters for configuration of the interface.
         *
         * @param config
         */
        virtual void setTargetParameters(const Targets::Microchip::Avr::Avr8Bit::TargetParameters& config) = 0;

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
         *
         * If the debugWire interface has been selected - this function should throw a DebugWirePhysicalInterfaceError
         * exception, in the event of a failure when activating the interface. The reason for this is to allow us the
         * chance to check the DWEN fuse bit, via an ISP interface. See Avr8::activate() for more.
         */
        virtual void activate() = 0;

        /**
         * Should deactivate the physical interface between the debug tool and the AVR8 target.
         */
        virtual void deactivate() = 0;

        /**
         * Should retrieve the AVR8 target signature of the AVR8 target.
         *
         * This method may invoke stop(), as the target may be required to be in a halted state before the signature
         * can be read.
         *
         * @return
         */
        virtual Targets::Microchip::Avr::TargetSignature getDeviceId() = 0;

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
         * Should update the program counter value on the target.
         *
         * @param programCounter
         */
        virtual void setProgramCounter(std::uint32_t programCounter) = 0;

        /**
         * Should read the requested registers from the target.
         *
         * @param descriptors
         *  A collection of register descriptors, for the registers to be read.
         *
         * @return
         */
        virtual Targets::TargetRegisters readRegisters(const Targets::TargetRegisterDescriptors& descriptors) = 0;

        /**
         * Should update the value of the given registers.
         *
         * @param registers
         */
        virtual void writeRegisters(const Targets::TargetRegisters& registers) = 0;

        /**
         * Should read memory from the target, for the given memory type.
         *
         * @param memoryType
         * @param startAddress
         * @param bytes
         * @param excludedAddressRanges
         * @return
         */
        virtual Targets::TargetMemoryBuffer readMemory(
            Targets::TargetMemoryType memoryType,
            std::uint32_t startAddress,
            std::uint32_t bytes,
            const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges = {}
        ) = 0;

        /**
         * Should write memory to the target, for a given memory type.
         *
         * @param memoryType
         * @param startAddress
         * @param buffer
         */
        virtual void writeMemory(
            Targets::TargetMemoryType memoryType,
            std::uint32_t startAddress,
            const Targets::TargetMemoryBuffer& buffer
        ) = 0;

        /**
         * Should erase the target's entire program memory, or a specific section where applicable.
         *
         * @param section
         *  The section to erase, or std::nullopt to erase the entire program memory.
         */
        virtual void eraseProgramMemory(
            std::optional<Targets::Microchip::Avr::Avr8Bit::ProgramMemorySection> section = std::nullopt
        ) = 0;

        /**
         * Should obtain the current target state.
         *
         * @return
         */
        virtual Targets::TargetState getTargetState() = 0;

        /**
         * Should prepare the debug interface for programming the target.
         */
        virtual void enableProgrammingMode() = 0;

        /**
         * Should prepare the debug interface for resuming debugging operations after a programming session.
         */
        virtual void disableProgrammingMode() = 0;
    };
}
