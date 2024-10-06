#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <functional>

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

#include "TriggerModule/TriggerModule.hpp"
#include "TriggerModule/TriggerDescriptor.hpp"

#include "src/Helpers/Expected.hpp"

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

        std::uint16_t getHardwareBreakpointCount() override;
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

        std::unordered_map<TriggerModule::TriggerIndex, TriggerModule::TriggerDescriptor> triggerDescriptorsByIndex;
        std::unordered_set<TriggerModule::TriggerIndex> allocatedTriggerIndices;
        std::unordered_map<Targets::TargetMemoryAddress, TriggerModule::TriggerIndex> triggerIndicesByBreakpointAddress;

        std::vector<DebugModule::HartIndex> discoverHartIndices();
        std::unordered_map<TriggerModule::TriggerIndex, TriggerModule::TriggerDescriptor> discoverTriggers();

        DebugModule::Registers::ControlRegister readDebugModuleControlRegister();
        DebugModule::Registers::StatusRegister readDebugModuleStatusRegister();
        DebugModule::Registers::AbstractControlStatusRegister readDebugModuleAbstractControlStatusRegister();
        Registers::DebugControlStatusRegister readDebugControlStatusRegister();

        void enableDebugModule();
        void disableDebugModule();

        Expected<RegisterValue, DebugModule::AbstractCommandError> tryReadCpuRegister(RegisterNumber number);
        Expected<RegisterValue, DebugModule::AbstractCommandError> tryReadCpuRegister(
            Registers::CpuRegisterNumber number
        );
        RegisterValue readCpuRegister(RegisterNumber number);
        RegisterValue readCpuRegister(Registers::CpuRegisterNumber number);

        DebugModule::AbstractCommandError tryWriteCpuRegister(RegisterNumber number, RegisterValue value);
        DebugModule::AbstractCommandError tryWriteCpuRegister(Registers::CpuRegisterNumber number, RegisterValue value);
        void writeCpuRegister(RegisterNumber number, RegisterValue value);
        void writeCpuRegister(Registers::CpuRegisterNumber number, RegisterValue value);

        void writeDebugModuleControlRegister(const DebugModule::Registers::ControlRegister& controlRegister);
        void writeDebugControlStatusRegister(const Registers::DebugControlStatusRegister& controlRegister);

        DebugModule::AbstractCommandError tryExecuteAbstractCommand(
            const DebugModule::Registers::AbstractCommandRegister& abstractCommandRegister
        );
        void executeAbstractCommand(const DebugModule::Registers::AbstractCommandRegister& abstractCommandRegister);

        Targets::TargetMemoryAddress alignMemoryAddress(
            Targets::TargetMemoryAddress address,
            Targets::TargetMemoryAddress alignTo
        );
        Targets::TargetMemorySize alignMemorySize(Targets::TargetMemorySize size, Targets::TargetMemorySize alignTo);

        std::optional<std::reference_wrapper<const TriggerModule::TriggerDescriptor>> getAvailableTrigger();
        void clearTrigger(const TriggerModule::TriggerDescriptor& triggerDescriptor);
    };
}
