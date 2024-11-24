#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <functional>

#include "src/DebugToolDrivers/TargetInterfaces/RiscV/RiscVDebugInterface.hpp"

#include "DebugTransportModuleInterface.hpp"
#include "DebugTranslatorConfig.hpp"
#include "DebugModuleDescriptor.hpp"

#include "src/Targets/RiscV/TargetDescriptionFile.hpp"
#include "src/Targets/RiscV/RiscVTargetConfig.hpp"
#include "src/Targets/RiscV/Opcodes/Opcode.hpp"
#include "src/Targets/TargetMemory.hpp"

#include "Common.hpp"
#include "Registers/CpuRegisterNumbers.hpp"
#include "Registers/DebugControlStatusRegister.hpp"

#include "DebugModule/DebugModule.hpp"
#include "DebugModule/Registers/ControlRegister.hpp"
#include "DebugModule/Registers/StatusRegister.hpp"
#include "DebugModule/Registers/AbstractControlStatusRegister.hpp"
#include "DebugModule/Registers/AbstractCommandRegister.hpp"
#include "DebugModule/Registers/RegisterAccessControlField.hpp"

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
            const DebugTranslatorConfig& config,
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

    private:
        static constexpr auto DEBUG_MODULE_RESPONSE_DELAY = std::chrono::microseconds{10};
        static constexpr auto WORD_BYTE_SIZE = ::Targets::TargetMemorySize{4};

        DebugTransportModuleInterface& dtmInterface;
        const DebugTranslatorConfig& config;

        const ::Targets::RiscV::TargetDescriptionFile& targetDescriptionFile;
        const ::Targets::RiscV::RiscVTargetConfig& targetConfig;

        DebugModuleDescriptor debugModuleDescriptor = {};

        DebugModule::HartIndex selectedHartIndex = 0;
        DebugModule::MemoryAccessStrategy memoryAccessStrategy = DebugModule::MemoryAccessStrategy::ABSTRACT_COMMAND;
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

        void initDebugControlStatusRegister();

        Expected<RegisterValue, DebugModule::AbstractCommandError> tryReadCpuRegister(
            RegisterNumber number,
            const DebugModule::Registers::RegisterAccessControlField::Flags& flags = {}
        );
        Expected<RegisterValue, DebugModule::AbstractCommandError> tryReadCpuRegister(
            Registers::CpuRegisterNumber number,
            const DebugModule::Registers::RegisterAccessControlField::Flags& flags = {}
        );
        RegisterValue readCpuRegister(
            RegisterNumber number,
            const DebugModule::Registers::RegisterAccessControlField::Flags& flags = {}
        );
        RegisterValue readCpuRegister(
            Registers::CpuRegisterNumber number,
            const DebugModule::Registers::RegisterAccessControlField::Flags& flags = {}
        );

        DebugModule::AbstractCommandError tryWriteCpuRegister(
            RegisterNumber number,
            RegisterValue value,
            const DebugModule::Registers::RegisterAccessControlField::Flags& flags = {}
        );
        DebugModule::AbstractCommandError tryWriteCpuRegister(
            Registers::CpuRegisterNumber number,
            RegisterValue value,
            const DebugModule::Registers::RegisterAccessControlField::Flags& flags = {}
        );
        void writeCpuRegister(
            RegisterNumber number,
            RegisterValue value,
            const DebugModule::Registers::RegisterAccessControlField::Flags& flags = {}
        );
        void writeCpuRegister(
            Registers::CpuRegisterNumber number,
            RegisterValue value,
            const DebugModule::Registers::RegisterAccessControlField::Flags& flags = {}
        );

        void writeDebugModuleControlRegister(const DebugModule::Registers::ControlRegister& controlRegister);
        void writeDebugControlStatusRegister(const Registers::DebugControlStatusRegister& controlRegister);

        void clearAbstractCommandError();
        DebugModule::AbstractCommandError tryExecuteAbstractCommand(
            const DebugModule::Registers::AbstractCommandRegister& abstractCommandRegister
        );
        void executeAbstractCommand(const DebugModule::Registers::AbstractCommandRegister& abstractCommandRegister);

        DebugModule::MemoryAccessStrategy determineMemoryAccessStrategy();

        Targets::TargetMemoryAddress alignMemoryAddress(
            Targets::TargetMemoryAddress address,
            Targets::TargetMemoryAddress alignTo
        );
        Targets::TargetMemorySize alignMemorySize(Targets::TargetMemorySize size, Targets::TargetMemorySize alignTo);

        Targets::TargetMemoryBuffer readMemoryViaAbstractCommand(
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemorySize bytes
        );
        void writeMemoryViaAbstractCommand(
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemoryBufferSpan buffer
        );

        Targets::TargetMemoryBuffer readMemoryViaProgramBuffer(
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemorySize bytes
        );
        void writeMemoryViaProgramBuffer(
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemoryBufferSpan buffer
        );

        void writeProgramBuffer(std::span<const Targets::RiscV::Opcodes::Opcode> opcodes);

        std::optional<std::reference_wrapper<const TriggerModule::TriggerDescriptor>> getAvailableTrigger();
        void clearTrigger(const TriggerModule::TriggerDescriptor& triggerDescriptor);

        struct PreservedCpuRegister
        {
            const Registers::CpuRegisterNumber registerNumber;
            const RegisterValue value;

            PreservedCpuRegister(
                Registers::CpuRegisterNumber registerNumber,
                RegisterValue value,
                DebugTranslator& debugTranslator
            );

            PreservedCpuRegister(
                Registers::CpuRegisterNumber registerNumber,
                DebugTranslator& debugTranslator
            );

            PreservedCpuRegister(const PreservedCpuRegister& other) = delete;
            PreservedCpuRegister& operator = (const PreservedCpuRegister& other) = delete;

            PreservedCpuRegister(const PreservedCpuRegister&& other) = delete;
            PreservedCpuRegister& operator = (const PreservedCpuRegister&& other) = delete;

            void restore();
            void restoreOnce();

        private:
            DebugTranslator& debugTranslator;
            bool restored = false;
        };
    };
}
