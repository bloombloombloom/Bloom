#pragma once

#include <cstdint>
#include <span>
#include <string>

#include "src/DebugToolDrivers/TargetInterfaces/RiscV/RiscVDebugInterface.hpp"

#include "Protocols/WchLink/WchLinkInterface.hpp"

#include "WchLinkToolConfig.hpp"
#include "src/DebugToolDrivers/Protocols/RiscVDebugSpec/DebugTranslator.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"
#include "src/Targets/ProgramBreakpointRegistry.hpp"

#include "src/Targets/RiscV/Wch/TargetDescriptionFile.hpp"
#include "src/Targets/RiscV/ProgramBreakpoint.hpp"

namespace DebugToolDrivers::Wch
{
    /**
     * The RISC-V debug module cannot provide a complete implementation of the RiscVDebugInterface.
     *
     * This class combines the functionality of the RISC-V debug module (via the RiscVDebugTranslator), with the
     * WCH-Link-specific functionality, to provide a complete implementation.
     */
    class WchLinkDebugInterface
        : public TargetInterfaces::RiscV::RiscVDebugInterface
    {
    public:
        WchLinkDebugInterface(
            const WchLinkToolConfig& toolConfig,
            const Targets::RiscV::RiscVTargetConfig& targetConfig,
            const Targets::RiscV::TargetDescriptionFile& targetDescriptionFile,
            Protocols::WchLink::WchLinkInterface& wchLinkInterface,
            const DeviceInfo& toolInfo
        );

        void activate() override;
        void deactivate() override;

        std::string getDeviceId() override;

        Targets::TargetExecutionState getExecutionState() override;

        void stop() override;
        void run() override;
        void step() override;
        void reset() override;

        Targets::BreakpointResources getBreakpointResources() override;
        void setProgramBreakpoint(const Targets::TargetProgramBreakpoint& breakpoint) override;
        void removeProgramBreakpoint(const Targets::TargetProgramBreakpoint& breakpoint) override;

        Targets::TargetRegisterDescriptorAndValuePairs readCpuRegisters(
            const Targets::TargetRegisterDescriptors& descriptors
        ) override;
        void writeCpuRegisters(const Targets::TargetRegisterDescriptorAndValuePairs& registers) override;

        Targets::TargetMemoryBuffer readMemory(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemorySize bytes,
            const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges
        ) override;
        void writeMemory(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemoryBufferSpan buffer
        ) override;
        void eraseMemory(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor
        ) override;

        void enableProgrammingMode() override;
        void disableProgrammingMode() override;

        void applyAccessRestrictions(Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor) override;
        void applyAccessRestrictions(Targets::TargetRegisterDescriptor& registerDescriptor) override;

    private:
        const WchLinkToolConfig& toolConfig;
        const Targets::RiscV::RiscVTargetConfig& targetConfig;
        const Targets::RiscV::TargetDescriptionFile& targetDescriptionFile;

        Protocols::WchLink::WchLinkInterface& wchLinkInterface;
        DebugToolDrivers::Protocols::RiscVDebugSpec::DebugTranslator riscVTranslator;

        DeviceInfo toolInfo;

        const Targets::TargetAddressSpaceDescriptor sysAddressSpaceDescriptor;
        const Targets::TargetMemorySegmentDescriptor& mainProgramSegmentDescriptor;

        /**
         * The 'target activation' command returns a payload of 5 bytes.
         *
         * The last 4 bytes hold the WCH target variant ID. Given that the 'target activation' command appears to be
         * the only way to obtain this ID, we cache it via WchLinkInterface::cachedVariantId and return the cached
         * value in WchLinkInterface::getTargetId().
         */
        std::optional<WchTargetVariantId> cachedVariantId;
        std::optional<WchTargetId> cachedTargetId;

        Targets::ProgramBreakpointRegistryGeneric<Targets::RiscV::ProgramBreakpoint> softwareBreakpointRegistry;

        std::span<const unsigned char> flashProgramOpcodes;
        Targets::TargetMemorySize programmingBlockSize;

        void setSoftwareBreakpoint(const Targets::TargetProgramBreakpoint& breakpoint);
        void clearSoftwareBreakpoint(const Targets::TargetProgramBreakpoint& breakpoint);

        void writeProgramMemoryPartialBlock(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemoryBufferSpan buffer
        );
        void writeProgramMemoryFullBlock(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemoryBufferSpan buffer
        );
        bool fullBlockWriteCompatible(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddress startAddress
        );

        static std::span<const unsigned char> getFlashProgramOpcodes(const std::string& key);
    };
}
