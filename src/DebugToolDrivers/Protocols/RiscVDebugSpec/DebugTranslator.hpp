#pragma once

#include <cstdint>
#include <string>

#include "src/DebugToolDrivers/TargetInterfaces/RiscV/RiscVDebugInterface.hpp"

#include "DebugTransportModuleInterface.hpp"

#include "src/Targets/RiscV/TargetDescriptionFile.hpp"
#include "src/Targets/RiscV/RiscVTargetConfig.hpp"

#include "RiscVGeneric.hpp"
#include "Registers/CpuRegisterNumbers.hpp"
#include "Registers/DebugControlStatusRegister.hpp"

#include "DebugModule/DebugModule.hpp"
#include "DebugModule/Registers/ControlRegister.hpp"
#include "DebugModule/Registers/StatusRegister.hpp"
#include "DebugModule/Registers/AbstractControlStatusRegister.hpp"
#include "DebugModule/Registers/AbstractCommandRegister.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebugSpec
{
    /**
     * Implementation of a RISC-V debug translator
     */
    class DebugTranslator: public ::DebugToolDrivers::TargetInterfaces::RiscV::RiscVDebugInterface
    {
    public:
        DebugTranslator(
            DebugTransportModuleInterface& dtmInterface,
            const ::Targets::RiscV::TargetDescriptionFile& targetDescriptionFile,
            const ::Targets::RiscV::RiscVTargetConfig& targetConfig
        );

        virtual ~DebugTranslator() = default;

        void init() override;
        void activate() override;
        void deactivate() override;

        Targets::TargetExecutionState getExecutionState() override;

        void stop() override;
        void run() override;
        void step() override;
        void reset() override;

        void setSoftwareBreakpoint(Targets::TargetMemoryAddress address) override;
        void clearSoftwareBreakpoint(Targets::TargetMemoryAddress address) override;

        void setHardwareBreakpoint(Targets::TargetMemoryAddress address) override;
        void clearHardwareBreakpoint(Targets::TargetMemoryAddress address) override;
        void clearAllBreakpoints() override;

        Targets::TargetRegisterDescriptorAndValuePairs readCpuRegisters(
            const Targets::TargetRegisterDescriptors& descriptors
        ) override;
        void writeCpuRegisters(const Targets::TargetRegisterDescriptorAndValuePairs& registers) override;

        Targets::TargetMemoryBuffer readMemory(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemorySize bytes,
            const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges = {}
        ) override;
        void writeMemory(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddress startAddress,
            const Targets::TargetMemoryBuffer& buffer
        ) override;

    private:
        DebugTransportModuleInterface& dtmInterface;

        const ::Targets::RiscV::TargetDescriptionFile& targetDescriptionFile;
        const ::Targets::RiscV::RiscVTargetConfig& targetConfig;

        std::vector<DebugModule::HartIndex> hartIndices;
        DebugModule::HartIndex selectedHartIndex = 0;

        std::vector<DebugModule::HartIndex> discoverHartIndices();

        DebugModule::Registers::ControlRegister readDebugModuleControlRegister();
        DebugModule::Registers::StatusRegister readDebugModuleStatusRegister();
        DebugModule::Registers::AbstractControlStatusRegister readDebugModuleAbstractControlStatusRegister();
        Registers::DebugControlStatusRegister readDebugControlStatusRegister();

        void enableDebugModule();
        void disableDebugModule();

        RegisterValue readCpuRegister(RegisterNumber number);
        void writeCpuRegister(RegisterNumber number, RegisterValue value);

        void writeDebugModuleControlRegister(const DebugModule::Registers::ControlRegister& controlRegister);
        void writeDebugControlStatusRegister(const Registers::DebugControlStatusRegister& controlRegister);

        void executeAbstractCommand(const DebugModule::Registers::AbstractCommandRegister& abstractCommandRegister);

        Targets::TargetMemoryAddress alignMemoryAddress(
            Targets::TargetMemoryAddress address,
            Targets::TargetMemoryAddress alignTo
        );
        Targets::TargetMemorySize alignMemorySize(Targets::TargetMemorySize size, Targets::TargetMemorySize alignTo);
    };
}
