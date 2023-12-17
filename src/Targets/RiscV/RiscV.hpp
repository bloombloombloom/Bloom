#pragma once

#include <cstdint>
#include <set>
#include <map>

#include "src/Targets/Target.hpp"
#include "src/DebugToolDrivers/DebugTool.hpp"

#include "TargetDescription/TargetDescriptionFile.hpp"

#include "src/DebugToolDrivers/TargetInterfaces/RiscV/RiscVDebugInterface.hpp"
#include "src/DebugToolDrivers/TargetInterfaces/RiscV/RiscVProgramInterface.hpp"

#include "src/Targets/RiscV/RiscVGeneric.hpp"
#include "src/Targets/RiscV/Registers/RegisterNumbers.hpp"
#include "src/Targets/RiscV/Registers/DebugControlStatusRegister.hpp"

#include "src/Targets/RiscV/DebugModule/DebugModule.hpp"
#include "src/Targets/RiscV/DebugModule/Registers/ControlRegister.hpp"
#include "src/Targets/RiscV/DebugModule/Registers/StatusRegister.hpp"
#include "src/Targets/RiscV/DebugModule/Registers/AbstractControlStatusRegister.hpp"
#include "src/Targets/RiscV/DebugModule/Registers/AbstractCommandRegister.hpp"

#include "RiscVRegisterDescriptor.hpp"

namespace Targets::RiscV
{
    class RiscV: public Target
    {
    public:
        explicit RiscV(
            const TargetConfig& targetConfig,
            TargetDescription::TargetDescriptionFile&& targetDescriptionFile
        );

        /*
         * The functions below implement the Target interface for RISC-V targets.
         *
         * See the Targets::Target abstract class for documentation on the expected behaviour of
         * each function.
         */

        /**
         * All RISC-V compatible debug tools must provide a valid RiscVDebugInterface.
         *
         * @param debugTool
         * @return
         */
        bool supportsDebugTool(DebugTool* debugTool) override;

        void setDebugTool(DebugTool* debugTool) override;

        void activate() override;
        void deactivate() override;

        TargetDescriptor getDescriptor() override;

        void run(std::optional<TargetMemoryAddress> toAddress = std::nullopt) override;
        void stop() override;
        void step() override;
        void reset() override;

        void setSoftwareBreakpoint(TargetMemoryAddress address) override;
        void removeSoftwareBreakpoint(TargetMemoryAddress address) override;

        void setHardwareBreakpoint(TargetMemoryAddress address) override;
        void removeHardwareBreakpoint(TargetMemoryAddress address) override;
        void clearAllBreakpoints() override;

        TargetRegisters readRegisters(const TargetRegisterDescriptorIds& descriptorIds) override;
        void writeRegisters(const TargetRegisters& registers) override;

        TargetMemoryBuffer readMemory(
            TargetMemoryType memoryType,
            TargetMemoryAddress startAddress,
            TargetMemorySize bytes,
            const std::set<TargetMemoryAddressRange>& excludedAddressRanges = {}
        ) override;
        void writeMemory(
            TargetMemoryType memoryType,
            TargetMemoryAddress startAddress,
            const TargetMemoryBuffer& buffer
        ) override;
        void eraseMemory(TargetMemoryType memoryType) override;

        TargetState getState() override;

        TargetMemoryAddress getProgramCounter() override;
        void setProgramCounter(TargetMemoryAddress programCounter) override;

        TargetStackPointer getStackPointer() override;

        std::map<int, TargetPinState> getPinStates(int variantId) override;
        void setPinState(
            const TargetPinDescriptor& pinDescriptor,
            const TargetPinState& state
        ) override;

        void enableProgrammingMode() override;

        void disableProgrammingMode() override;

        bool programmingModeEnabled() override;

    protected:
        TargetDescription::TargetDescriptionFile targetDescriptionFile;

        std::map<TargetRegisterDescriptorId, RiscVRegisterDescriptor> registerDescriptorsById;

        RiscVRegisterDescriptor stackPointerRegisterDescriptor;

        DebugToolDrivers::TargetInterfaces::RiscV::RiscVDebugInterface* riscVDebugInterface = nullptr;
        DebugToolDrivers::TargetInterfaces::RiscV::RiscVProgramInterface* riscVProgramInterface = nullptr;

        std::set<DebugModule::HartIndex> hartIndices;
        DebugModule::HartIndex selectedHartIndex = 0;

        void loadRegisterDescriptors();

        std::set<DebugModule::HartIndex> discoverHartIndices();

        DebugModule::Registers::ControlRegister readDebugModuleControlRegister();
        DebugModule::Registers::StatusRegister readDebugModuleStatusRegister();
        DebugModule::Registers::AbstractControlStatusRegister readDebugModuleAbstractControlStatusRegister();

        Registers::DebugControlStatusRegister readDebugControlStatusRegister();

        void enableDebugModule();
        void disableDebugModule();

        RegisterValue readRegister(RegisterNumber number);
        RegisterValue readRegister(Registers::RegisterNumber number);
        void writeRegister(RegisterNumber number, RegisterValue value);
        void writeRegister(Registers::RegisterNumber number, RegisterValue value);

        void writeDebugModuleControlRegister(const DebugModule::Registers::ControlRegister& controlRegister);

        void writeDebugControlStatusRegister(const Registers::DebugControlStatusRegister& controlRegister);

        void executeAbstractCommand(const DebugModule::Registers::AbstractCommandRegister& abstractCommandRegister);

        TargetMemoryAddress alignMemoryAddress(TargetMemoryAddress address, TargetMemoryAddress alignTo);
        TargetMemorySize alignMemorySize(TargetMemorySize size, TargetMemorySize alignTo);
    };
}
