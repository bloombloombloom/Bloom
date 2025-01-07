#pragma once

#include <cstdint>
#include <queue>
#include <utility>
#include <optional>

#include "src/Targets/Target.hpp"
#include "src/DebugToolDrivers/DebugTool.hpp"

#include "src/DebugToolDrivers/TargetInterfaces/Microchip/Avr8/Avr8DebugInterface.hpp"
#include "src/DebugToolDrivers/TargetInterfaces/Microchip/Avr8/AvrIspInterface.hpp"

#include "Family.hpp"
#include "GpioPadDescriptor.hpp"
#include "ProgramMemorySection.hpp"
#include "ProgrammingSession.hpp"

#include "src/Targets/Microchip/Avr8/Fuse.hpp"
#include "src/Targets/TargetPhysicalInterface.hpp"
#include "src/Targets/TargetRegisterDescriptor.hpp"
#include "src/Targets/TargetBitFieldDescriptor.hpp"
#include "src/Targets/TargetPadDescriptor.hpp"
#include "src/Targets/TargetBreakpoint.hpp"

#include "TargetDescriptionFile.hpp"

#include "Avr8TargetConfig.hpp"

namespace Targets::Microchip::Avr8
{
    class Avr8: public Target
    {
    public:
        explicit Avr8(const TargetConfig& targetConfig, TargetDescriptionFile&& targetDescriptionFile);

        /*
         * The functions below implement the Target interface for AVR8 targets.
         *
         * See the Targets::Target abstract class for documentation on the expected behaviour of
         * each function.
         */

        /**
         * All AVR8 compatible debug tools must provide a valid Avr8Interface.
         *
         * @param debugTool
         * @return
         */
        bool supportsDebugTool(DebugTool* debugTool) override;

        void setDebugTool(DebugTool* debugTool) override;

        void activate() override;
        void deactivate() override;

        void postActivate() override;

        TargetDescriptor targetDescriptor() override;

        void run(std::optional<TargetMemoryAddress> toAddress) override;
        void stop() override;
        void step() override;
        void reset() override;

        void setProgramBreakpoint(const TargetProgramBreakpoint& breakpoint) override;
        void removeProgramBreakpoint(const TargetProgramBreakpoint& breakpoint) override;

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

        TargetGpioPadDescriptorAndStatePairs getGpioPadStates(const TargetPadDescriptors& padDescriptors) override;
        void setGpioPadState(const TargetPadDescriptor& padDescriptor, const TargetGpioPadState& state) override;

        void enableProgrammingMode() override;
        void disableProgrammingMode() override;
        bool programmingModeEnabled() override;

        std::string passthroughCommandHelpText() override;
        std::optional<PassthroughResponse> invokePassthroughCommand(const PassthroughCommand& command) override;

    protected:
        DebugToolDrivers::TargetInterfaces::TargetPowerManagementInterface* targetPowerManagementInterface = nullptr;
        DebugToolDrivers::TargetInterfaces::Microchip::Avr8::Avr8DebugInterface* avr8DebugInterface = nullptr;
        DebugToolDrivers::TargetInterfaces::Microchip::Avr8::AvrIspInterface* avrIspInterface = nullptr;

        Avr8TargetConfig targetConfig;
        TargetDescriptionFile targetDescriptionFile;

        TargetAddressSpaceDescriptor programAddressSpaceDescriptor;
        TargetAddressSpaceDescriptor dataAddressSpaceDescriptor;
        TargetAddressSpaceDescriptor fuseAddressSpaceDescriptor;

        TargetMemorySegmentDescriptor programMemorySegmentDescriptor;
        TargetMemorySegmentDescriptor ramMemorySegmentDescriptor;
        TargetMemorySegmentDescriptor ioMemorySegmentDescriptor;
        TargetMemorySegmentDescriptor fuseMemorySegmentDescriptor;

        TargetSignature signature;
        Family family;

        bool activated = false;

        std::set<TargetPhysicalInterface> physicalInterfaces;

        std::vector<TargetPeripheralDescriptor> gpioPortPeripheralDescriptors;
        std::map<TargetPadId, GpioPadDescriptor> gpioPadDescriptorsByPadId;

        /**
         * The stack pointer register on AVR8 targets can take several forms:
         *
         *  1. A single 8 or 16-bit register in the CPU peripheral, with key "sp"
         *  2. Two 8-bit registers for high and low bytes in a 16-bit stack pointer, in the CPU peripheral, with keys
         *    "spl" and "sph"
         *  3. A single 8-bit low byte register for an 8-bit stack pointer, in the CPU peripheral. This is similar
         *     to 1, but with key "spl"
         */
        std::optional<TargetRegisterDescriptor> spRegisterDescriptor;
        std::optional<TargetRegisterDescriptor> spLowRegisterDescriptor;
        std::optional<TargetRegisterDescriptor> spHighRegisterDescriptor;

        /**
         * On some AVR8 targets, like the ATmega328P, a cleared fuse bit means the fuse is "programmed" (enabled).
         * And a set bit means the fuse is "un-programmed" (disabled). But on others, like the ATmega4809, it's the
         * other way around (set bit == enabled, cleared bit == disabled).
         *
         * The FuseEnableStrategy specifies the strategy of enabling a fuse. It's extracted from the TDF.
         * See TargetDescription::getFuseEnableStrategy() for more.
         */
        FuseEnableStrategy fuseEnableStrategy;

        std::optional<ProgrammingSession> activeProgrammingSession = std::nullopt;

        static std::map<TargetPadId, GpioPadDescriptor> generateGpioPadDescriptorMapping(
            const std::vector<TargetPeripheralDescriptor>& portPeripheralDescriptors
        );

        TargetMemoryBuffer readRegister(const TargetRegisterDescriptor& descriptor);
        void writeRegister(const TargetRegisterDescriptor& descriptor, const TargetMemoryBuffer& value) ;

        void applyDebugInterfaceAccessRestrictions(
            TargetRegisterGroupDescriptor& groupDescriptor,
            const TargetAddressSpaceDescriptor& addressSpaceDescriptor
        );

        BreakpointResources getBreakpointResources();

        /**
         * Checks if a particular fuse is enabled in the given fuse byte value. Takes the target's fuse enable strategy
         * into account.
         *
         * @param bitFieldDescriptor
         * @param value
         *
         * @return
         */
        bool isFuseEnabled(const TargetBitFieldDescriptor& bitFieldDescriptor, FuseValue value) const;

        /**
         * Enables/disables a fuse within the given fuse byte, using the target's fuse enable strategy.
         *
         * @param bitFieldDescriptor
         * @param value
         * @param enabled
         *
         * @return
         *  The updated fuse byte value.
         */
        FuseValue setFuseEnabled(
            const TargetBitFieldDescriptor& bitFieldDescriptor,
            FuseValue value,
            bool enabled
        ) const;

        /**
         * Updates the debugWIRE enable (DWEN) fuse bit on the AVR target.
         *
         * @param enable
         *  True to enable the fuse, false to disable it.
         */
        void updateDwenFuseBit(bool enable);

        /**
         * Updates the On-chip debug enable (OCDEN) fuse bit on the AVR target.
         *
         * @param enable
         *  True to enable the fuse, false to disable it.
         */
        void updateOcdenFuseBit(bool enable);

        /**
         * Updates the "Preserve EEPROM" (EESAVE) fuse bit on the AVR target.
         *
         * @param enable
         *  True to enable the fuse, false to disable it.
         *
         * @return
         *  True if the fuse bit was updated. False if the fuse bit was already set to the desired value.
         */
        bool updateEesaveFuseBit(bool enable);
    };
}
