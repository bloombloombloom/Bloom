#pragma once

#include <cstdint>

#include "src/Targets/Target.hpp"
#include "src/DebugToolDrivers/DebugTool.hpp"

#include "src/DebugToolDrivers/TargetInterfaces/RiscV/RiscVDebugInterface.hpp"

#include "src/Targets/RiscV/DebugModule/DebugModule.hpp"
#include "src/Targets/RiscV/DebugModule/Registers/ControlRegister.hpp"
#include "src/Targets/RiscV/DebugModule/Registers/StatusRegister.hpp"

namespace Targets::RiscV
{
    class RiscV: public Target
    {
    public:
        explicit RiscV(const TargetConfig& targetConfig);

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

        void writeRegisters(TargetRegisters registers) override;
        TargetRegisters readRegisters(const Targets::TargetRegisterDescriptorIds& descriptorIds) override;

        TargetMemoryBuffer readMemory(
            TargetMemoryType memoryType,
            TargetMemoryAddress startAddress,
            TargetMemorySize bytes,
            const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges = {}
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
        DebugToolDrivers::TargetInterfaces::RiscV::RiscVDebugInterface* riscVDebugInterface = nullptr;
        std::string name;

        DebugModule::Registers::ControlRegister readControlRegister();
        DebugModule::Registers::StatusRegister readStatusRegister();

        void writeControlRegister(const DebugModule::Registers::ControlRegister& controlRegister);
    };
}
