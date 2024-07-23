#pragma once

#include <cstdint>
#include <set>
#include <map>

#include "src/Targets/Target.hpp"
#include "src/DebugToolDrivers/DebugTool.hpp"

#include "RiscVTargetConfig.hpp"
#include "TargetDescriptionFile.hpp"

#include "src/DebugToolDrivers/TargetInterfaces/RiscV/RiscVDebugInterface.hpp"
#include "src/DebugToolDrivers/TargetInterfaces/RiscV/RiscVProgramInterface.hpp"
#include "src/DebugToolDrivers/TargetInterfaces/RiscV/RiscVIdentificationInterface.hpp"

namespace Targets::RiscV
{
    class RiscV: public Target
    {
    public:
        RiscV(const TargetConfig& targetConfig, TargetDescriptionFile&& targetDescriptionFile);

        /*
         * The functions below implement the Target interface for RISC-V targets.
         *
         * See the Targets::Target abstract class for documentation on the expected behaviour of
         * each function.
         */

        bool supportsDebugTool(DebugTool* debugTool) override;
        void setDebugTool(DebugTool* debugTool) override;

        void activate() override;
        void deactivate() override;

        TargetDescriptor targetDescriptor() override;

        void run(std::optional<TargetMemoryAddress> toAddress = std::nullopt) override;
        void stop() override;
        void step() override;
        void reset() override;

        void setSoftwareBreakpoint(TargetMemoryAddress address) override;
        void removeSoftwareBreakpoint(TargetMemoryAddress address) override;

        void setHardwareBreakpoint(TargetMemoryAddress address) override;
        void removeHardwareBreakpoint(TargetMemoryAddress address) override;
        void clearAllBreakpoints() override;

        TargetRegisterDescriptorAndValuePairs readRegisters(const TargetRegisterDescriptors& descriptors) override;
        void writeRegisters(const TargetRegisterDescriptorAndValuePairs& registers) override;

        TargetMemoryBuffer readMemory(
            const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            TargetMemoryAddress startAddress,
            TargetMemorySize bytes,
            const std::set<TargetMemoryAddressRange>& excludedAddressRanges = {}
        ) override;
        void writeMemory(
            const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            TargetMemoryAddress startAddress,
            const TargetMemoryBuffer& buffer
        ) override;
        bool isProgramMemory(
            const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            TargetMemoryAddress startAddress,
            TargetMemorySize size
        ) override;
        void eraseMemory(
            const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const TargetMemorySegmentDescriptor& memorySegmentDescriptor
        ) override;

        TargetExecutionState getExecutionState() override;

        TargetMemoryAddress getProgramCounter() override;
        void setProgramCounter(TargetMemoryAddress programCounter) override;

        TargetStackPointer getStackPointer() override;
        void setStackPointer(TargetStackPointer stackPointer) override;

        TargetGpioPinDescriptorAndStatePairs getGpioPinStates(
            const TargetPinoutDescriptor& pinoutDescriptor
        ) override;
        void setGpioPinState(const TargetPinDescriptor& pinDescriptor, const TargetGpioPinState& state) override;

        void enableProgrammingMode() override;

        void disableProgrammingMode() override;

        bool programmingModeEnabled() override;

    protected:
        RiscVTargetConfig targetConfig;
        TargetDescriptionFile targetDescriptionFile;

        DebugToolDrivers::TargetInterfaces::RiscV::RiscVDebugInterface* riscVDebugInterface = nullptr;
        DebugToolDrivers::TargetInterfaces::RiscV::RiscVProgramInterface* riscVProgramInterface = nullptr;
        DebugToolDrivers::TargetInterfaces::RiscV::RiscVIdentificationInterface* riscVIdInterface = nullptr;

        /*
         * On RISC-V targets, CPU registers are typically only accessible via the debug module (we can't access them
         * via the system address space). So we use abstract commands to access these registers. This means we have to
         * address these registers via their register numbers, as defined in the RISC-V debug spec.
         *
         * We effectively treat register numbers as a separate address space, with an addressable unit size of 4 bytes.
         * The `cpuRegisterAddressSpaceDescriptor` member holds the descriptor for this address space.
         *
         * TODO: review this. This address space is specific to the RISC-V debug spec, but some debug tools may
         *       implement their own debug translator in firmware, and then provide a higher-level API to access the
         *       same registers. In that case, this address space may not be relevant. This may need to be moved.
         *       ATM all RISC-V debug tools supported by Bloom provide a DTM interface, so we use our own debug
         *       translator driver and this address space is, in fact, relevant. I will deal with this when it
         *       becomes a problem.
         */
        TargetAddressSpaceDescriptor cpuRegisterAddressSpaceDescriptor;
        const TargetMemorySegmentDescriptor& csrMemorySegmentDescriptor;
        const TargetMemorySegmentDescriptor& gprMemorySegmentDescriptor;

        TargetPeripheralDescriptor cpuPeripheralDescriptor;
        const TargetRegisterGroupDescriptor& csrGroupDescriptor;
        const TargetRegisterGroupDescriptor& gprGroupDescriptor;
        const TargetRegisterDescriptor& pcRegisterDescriptor;
        const TargetRegisterDescriptor& spRegisterDescriptor;

        /*
         * The "system" address space is the main address space on RISC-V targets.
         */
        TargetAddressSpaceDescriptor sysAddressSpaceDescriptor;

        const TargetMemorySegmentDescriptor& resolveRegisterMemorySegmentDescriptor(
            const TargetRegisterDescriptor& regDescriptor,
            const TargetAddressSpaceDescriptor& addressSpaceDescriptor
        );

        static TargetAddressSpaceDescriptor generateCpuRegisterAddressSpaceDescriptor();
        static TargetPeripheralDescriptor generateCpuPeripheralDescriptor(
            const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const TargetMemorySegmentDescriptor& csrMemorySegmentDescriptor,
            const TargetMemorySegmentDescriptor& gprMemorySegmentDescriptor
        );
    };
}
