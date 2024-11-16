#pragma once

#include <cstdint>
#include <set>
#include <vector>
#include <optional>

#include "src/Targets/Microchip/AVR8/TargetSignature.hpp"
#include "src/Targets/Microchip/AVR8/Family.hpp"
#include "src/Targets/Microchip/AVR8/ProgramMemorySection.hpp"

#include "src/Targets/TargetDescriptor.hpp"
#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"
#include "src/Targets/TargetState.hpp"
#include "src/Targets/TargetRegisterDescriptor.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace DebugToolDrivers::TargetInterfaces::Microchip::Avr8
{
    /**
     * Interfacing with an AVR8 target for debugging operations can vary significantly, depending on the debug tool
     * being used. Some debug tools employ different protocols.
     *
     * This class describes the interface required for interfacing with AVR8 targets, for debugging operations.
     *
     * Each debug tool that supports interfacing with AVR8 targets must provide an implementation
     * of this interface class. For example, the Atmel-ICE provides the EdbgAvr8Interface implementation for
     * interfacing with AVR8 targets. See DebugToolDrivers::AtmelIce::getAvr8DebugInterface() and
     * DebugTool::getAvr8DebugInterface() for more on this.
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

        virtual void init() = 0;
        virtual void stop() = 0;
        virtual void run() = 0;
        virtual void runTo(Targets::TargetMemoryAddress address) = 0;
        virtual void step() = 0;
        virtual void reset() = 0;

        /**
         * Should activate the physical interface between the debug tool and the AVR8 target.
         *
         * If the debugWIRE interface has been selected - this function should throw a DebugWirePhysicalInterfaceError
         * exception, in the event of a failure when activating the interface. The reason for this is to allow us the
         * chance to check the DWEN fuse bit, via an ISP interface. See Avr8::activate() for more.
         */
        virtual void activate() = 0;

        /**
         * Should deactivate the physical interface between the debug tool and the AVR8 target.
         */
        virtual void deactivate() = 0;

        /**
         * We can specify access restrictions for individual registers in our TDFs, but this is only at a target
         * level - it does not account for any restrictions that the debug interface may be subject to.
         *
         * For example, EDBG debug tools cannot access fuse registers on JTAG targets, during a debug session. Those
         * registers can only be accessed during a programming session. This restriction is specific to the EDBG debug
         * interface.
         *
         * This function should communicate any access restrictions for the given register, which apply during a debug
         * session. It does not need to account for any access restrictions that only apply outside of a debug session.
         *
         * @param registerDescriptor
         *  The descriptor of the register.
         *
         * @param addressSpaceDescriptor
         *  The descriptor of the address space in which the register resides.
         *
         * @return
         */
        virtual Targets::TargetRegisterAccess getRegisterAccess(
            const Targets::TargetRegisterDescriptor& registerDescriptor,
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor
        ) = 0;

        /**
         * Should retrieve the AVR8 target signature of the AVR8 target.
         *
         * This method may invoke stop(), as the target may be required to be in a halted state before the signature
         * can be read.
         *
         * @return
         */
        virtual Targets::Microchip::Avr8::TargetSignature getDeviceId() = 0;

        virtual void setSoftwareBreakpoint(Targets::TargetMemoryAddress address) = 0;
        virtual void clearSoftwareBreakpoint(Targets::TargetMemoryAddress address) = 0;
        virtual void setHardwareBreakpoint(Targets::TargetMemoryAddress address) = 0;
        virtual void clearHardwareBreakpoint(Targets::TargetMemoryAddress address) = 0;
        virtual void clearAllBreakpoints() = 0;
        virtual Targets::TargetMemoryAddress getProgramCounter() = 0;
        virtual void setProgramCounter(Targets::TargetMemoryAddress programCounter) = 0;
        virtual Targets::TargetRegisterDescriptorAndValuePairs readRegisters(
            const Targets::TargetRegisterDescriptors& descriptors
        ) = 0;
        virtual void writeRegisters(const Targets::TargetRegisterDescriptorAndValuePairs& registers) = 0;
        virtual Targets::TargetMemoryBuffer readMemory(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemorySize bytes,
            const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges = {}
        ) = 0;
        virtual void writeMemory(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemoryBufferSpan buffer
        ) = 0;

        /**
         * Should erase the target's entire program memory, or a specific section where applicable.
         *
         * @param section
         *  The section to erase, or std::nullopt to erase the entire program memory.
         */
        virtual void eraseProgramMemory(
            std::optional<Targets::Microchip::Avr8::ProgramMemorySection> section = std::nullopt
        ) = 0;

        virtual void eraseChip() = 0;
        virtual Targets::TargetExecutionState getExecutionState() = 0;
        virtual void enableProgrammingMode() = 0;
        virtual void disableProgrammingMode() = 0;
    };
}
