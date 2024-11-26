#pragma once

#include <cstdint>
#include <span>
#include <string>

#include "src/DebugToolDrivers/TargetInterfaces/RiscV/RiscVDebugInterface.hpp"

#include "Protocols/WchLink/WchLinkInterface.hpp"

#include "WchLinkToolConfig.hpp"
#include "src/DebugToolDrivers/Protocols/RiscVDebugSpec/DebugTranslator.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/RiscV/Wch/TargetDescriptionFile.hpp"

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
            Protocols::WchLink::WchLinkInterface& wchLinkInterface
        );

        void activate() override;
        void deactivate() override;

        std::string getDeviceId() override;

        Targets::TargetExecutionState getExecutionState() override;

        void stop() override;
        void run() override;
        void step() override;
        void reset() override;

        void setSoftwareBreakpoint(Targets::TargetMemoryAddress address) override;
        void clearSoftwareBreakpoint(Targets::TargetMemoryAddress address) override;

        std::uint16_t getHardwareBreakpointCount() override;
        void setHardwareBreakpoint(Targets::TargetMemoryAddress address) override;
        void clearHardwareBreakpoint(Targets::TargetMemoryAddress address) override;
        void clearAllHardwareBreakpoints() override;

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

    private:
        static constexpr Targets::TargetMemorySize MAX_PARTIAL_BLOCK_WRITE_SIZE = 64;

        const WchLinkToolConfig& toolConfig;
        const Targets::RiscV::RiscVTargetConfig& targetConfig;
        const Targets::RiscV::TargetDescriptionFile& targetDescriptionFile;

        Protocols::WchLink::WchLinkInterface& wchLinkInterface;
        DebugToolDrivers::Protocols::RiscVDebugSpec::DebugTranslator riscVTranslator;

        /**
         * The 'target activation' command returns a payload of 5 bytes.
         *
         * The last 4 bytes hold the WCH target variant ID. Given that the 'target activation' command appears to be
         * the only way to obtain this ID, we cache it via WchLinkInterface::cachedVariantId and return the cached
         * value in WchLinkInterface::getTargetId().
         */
        std::optional<WchTargetVariantId> cachedVariantId;
        std::optional<WchTargetId> cachedTargetId;

        std::span<const unsigned char> flashProgramOpcodes;
        Targets::TargetMemorySize programmingBlockSize;

        void writeFlashMemory(Targets::TargetMemoryAddress startAddress, Targets::TargetMemoryBufferSpan buffer);
        void eraseFlashMemory();

        static std::span<const unsigned char> getFlashProgramOpcodes(const std::string& key);
    };
}
