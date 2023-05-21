#pragma once

#include <cstdint>
#include <queue>
#include <utility>
#include <optional>

#include "src/Targets/Target.hpp"
#include "src/DebugToolDrivers/DebugTool.hpp"

#include "src/DebugToolDrivers/TargetInterfaces/Microchip/AVR/AVR8/Avr8DebugInterface.hpp"
#include "src/DebugToolDrivers/TargetInterfaces/Microchip/AVR/AvrIspInterface.hpp"

#include "Family.hpp"
#include "TargetParameters.hpp"
#include "PadDescriptor.hpp"
#include "ProgramMemorySection.hpp"
#include "src/Targets/TargetRegister.hpp"

#include "TargetDescription/TargetDescriptionFile.hpp"

#include "Avr8TargetConfig.hpp"

namespace Bloom::Targets::Microchip::Avr::Avr8Bit
{
    class Avr8: public Target
    {
    public:
        explicit Avr8(const TargetConfig& targetConfig);

        /*
         * The functions below implement the Target interface for AVR8 targets.
         *
         * See the Bloom::Targets::Target abstract class for documentation on the expected behaviour of
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

        TargetDescriptor getDescriptor() override;

        void run(std::optional<TargetMemoryAddress> toAddress = std::nullopt) override;
        void stop() override;
        void step() override;
        void reset() override;

        void setBreakpoint(TargetProgramCounter address) override;
        void removeBreakpoint(TargetProgramCounter address) override;
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

        TargetProgramCounter getProgramCounter() override;
        void setProgramCounter(TargetProgramCounter programCounter) override;

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
        DebugToolDrivers::TargetInterfaces::TargetPowerManagementInterface* targetPowerManagementInterface = nullptr;
        DebugToolDrivers::TargetInterfaces::Microchip::Avr::Avr8::Avr8DebugInterface* avr8DebugInterface = nullptr;
        DebugToolDrivers::TargetInterfaces::Microchip::Avr::AvrIspInterface* avrIspInterface = nullptr;

        Avr8TargetConfig targetConfig;

        TargetDescription::TargetDescriptionFile targetDescriptionFile;

        TargetSignature signature;
        std::string name;
        Family family;

        TargetParameters targetParameters;

        std::set<PhysicalInterface> supportedPhysicalInterfaces;
        std::map<std::string, PadDescriptor> padDescriptorsByName;
        std::map<int, TargetVariant> targetVariantsById;

        TargetRegisterDescriptor stackPointerRegisterDescriptor;
        TargetRegisterDescriptor statusRegisterDescriptor;

        std::map<TargetRegisterDescriptorId, TargetRegisterDescriptor> targetRegisterDescriptorsById;

        std::map<TargetMemoryType, TargetMemoryDescriptor> targetMemoryDescriptorsByType;

        bool progModeEnabled = false;

        /**
         * Populates this->targetRegisterDescriptorsById with registers extracted from the TDF, as well as general
         * purpose and other CPU registers.
         */
        void loadTargetRegisterDescriptors();

        void loadTargetMemoryDescriptors();

        /**
         * Updates the debugWire enable (DWEN) fuse bit on the AVR target.
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
         * Resolves the program memory section from a program memory address.
         *
         * Currently unused, but will be needed soon.
         *
         * @param address
         * @return
         */
        ProgramMemorySection getProgramMemorySectionFromAddress(TargetMemoryAddress address);
    };
}
