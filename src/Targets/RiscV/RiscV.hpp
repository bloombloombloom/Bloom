#pragma once

#include <cstdint>
#include <set>
#include <map>

#include "src/Targets/Target.hpp"
#include "src/DebugToolDrivers/DebugTool.hpp"

#include "RiscVTargetConfig.hpp"
#include "TargetDescriptionFile.hpp"
#include "IsaDescriptor.hpp"

#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetRegisterGroupDescriptor.hpp"
#include "src/Targets/DynamicRegisterValue.hpp"

#include "src/DebugToolDrivers/TargetInterfaces/RiscV/RiscVDebugInterface.hpp"

namespace Targets::RiscV
{
    class RiscV: public Target
    {
    public:
        RiscV(const TargetConfig& targetConfig, const TargetDescriptionFile& targetDescriptionFile);

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

        void run(std::optional<TargetMemoryAddress> toAddress = std::nullopt) override;
        void stop() override;
        void step() override;
        void reset() override;

        TargetRegisterDescriptorAndValuePairs readRegisters(const TargetRegisterDescriptors& descriptors) override;
        void writeRegisters(const TargetRegisterDescriptorAndValuePairs& registers) override;

        TargetMemoryBuffer readMemory(
            const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            TargetMemoryAddress startAddress,
            TargetMemorySize bytes,
            const std::set<TargetMemoryAddressRange>& excludedAddressRanges
        ) override;
        void writeMemory(
            const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            TargetMemoryAddress startAddress,
            TargetMemoryBufferSpan buffer
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

        void enableProgrammingMode() override;
        void disableProgrammingMode() override;
        bool programmingModeEnabled() override;

    protected:
        RiscVTargetConfig targetConfig;
        TargetDescriptionFile targetDescriptionFile;
        IsaDescriptor isaDescriptor;

        DebugToolDrivers::TargetInterfaces::RiscV::RiscVDebugInterface* riscVDebugInterface = nullptr;

        TargetAddressSpaceDescriptor csrAddressSpaceDescriptor;
        const TargetMemorySegmentDescriptor& csrMemorySegmentDescriptor;
        TargetAddressSpaceDescriptor gprAddressSpaceDescriptor;
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

        bool programmingMode = false;

        TargetMemoryBuffer readRegister(const TargetRegisterDescriptor& descriptor);
        DynamicRegisterValue readRegisterDynamicValue(const TargetRegisterDescriptor& descriptor);
        void writeRegister(const DynamicRegisterValue& dynamicRegister);
        void writeRegister(const TargetRegisterDescriptor& descriptor, TargetMemoryBufferSpan value);
        void writeRegister(const TargetRegisterDescriptor& descriptor, std::uint64_t value);

        bool probeMemory(
            const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            TargetMemoryAddress address
        );

        void applyDebugInterfaceAccessRestrictions(TargetAddressSpaceDescriptor& addressSpaceDescriptor);
        void applyDebugInterfaceAccessRestrictions(TargetRegisterGroupDescriptor& registerGroupDescriptor);

        const TargetMemorySegmentDescriptor& resolveRegisterMemorySegmentDescriptor(
            const TargetRegisterDescriptor& regDescriptor,
            const TargetAddressSpaceDescriptor& addressSpaceDescriptor
        );

        static const TargetRegisterGroupDescriptor& generateGeneralPurposeRegisterGroupDescriptor(
            const IsaDescriptor& isaDescriptor,
            const TargetAddressSpaceDescriptor& gprAddressSpaceDescriptor,
            const TargetMemorySegmentDescriptor& gprMemorySegmentDescriptor,
            TargetPeripheralDescriptor& cpuPeripheralDescriptor
        );
    };
}
