#pragma once

#include <cstdint>
#include <queue>
#include <utility>
#include <optional>

#include "src/Targets/Microchip/AVR/Target.hpp"
#include "src/DebugToolDrivers/DebugTool.hpp"

#include "src/DebugToolDrivers/TargetInterfaces/Microchip/AVR/AVR8/Avr8DebugInterface.hpp"
#include "src/DebugToolDrivers/TargetInterfaces/Microchip/AVR/AvrIspInterface.hpp"

#include "Family.hpp"
#include "TargetParameters.hpp"
#include "PadDescriptor.hpp"
#include "ProgrammingSession.hpp"
#include "ProgramMemorySection.hpp"
#include "src/Targets/TargetRegister.hpp"

#include "TargetDescription/TargetDescriptionFile.hpp"

#include "Avr8TargetConfig.hpp"

namespace Bloom::Targets::Microchip::Avr::Avr8Bit
{
    class Avr8: public Target
    {
    public:
        explicit Avr8() = default;
        Avr8(std::string name, const TargetSignature& signature)
            : name(std::move(name))
            , Target(signature)
        {
            this->initFromTargetDescriptionFile();
        };

        /*
         * The functions below implement the Target interface for AVR8 targets.
         *
         * See the Bloom::Targets::Target interface class for documentation on the expected behaviour of
         * each function.
         */

        void preActivationConfigure(const TargetConfig& targetConfig) override;
        void postActivationConfigure() override;
        void postPromotionConfigure() override;

        void activate() override;
        void deactivate() override;

        /**
         * All AVR8 compatible debug tools must provide a valid Avr8Interface.
         *
         * @param debugTool
         * @return
         */
        bool isDebugToolSupported(DebugTool* debugTool) override {
            return debugTool->getAvr8DebugInterface() != nullptr;
        }

        void setDebugTool(DebugTool* debugTool) override {
            this->targetPowerManagementInterface = debugTool->getTargetPowerManagementInterface();
            this->avr8DebugInterface = debugTool->getAvr8DebugInterface();
            this->avrIspInterface = debugTool->getAvrIspInterface();
        }

        /**
         * Instances to this target class can be promoted. See Avr8::promote() method for more.
         *
         * @return
         */
        bool supportsPromotion() override {
            return true;
        }

        /**
         * Instances of this generic Avr8 target class will be promoted to a family specific class (see the Mega, Xmega
         * and Tiny classes for more).
         *
         * @return
         */
        std::unique_ptr<Targets::Target> promote() override;

        std::string getName() const override {
            return this->name;
        }

        TargetDescriptor getDescriptor() override;

        void run() override;
        void stop() override;
        void step() override;
        void reset() override;

        void setBreakpoint(TargetProgramCounter address) override;
        void removeBreakpoint(TargetProgramCounter address) override;
        void clearAllBreakpoints() override;

        void writeRegisters(TargetRegisters registers) override;
        TargetRegisters readRegisters(TargetRegisterDescriptors descriptors) override;

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

        TargetState getState() override;

        TargetProgramCounter getProgramCounter() override;
        TargetRegister getProgramCounterRegister();
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

        std::optional<Avr8TargetConfig> targetConfig;

        std::string name;
        std::optional<Family> family;
        std::optional<TargetDescription::TargetDescriptionFile> targetDescriptionFile;

        std::set<PhysicalInterface> supportedPhysicalInterfaces;

        std::optional<TargetParameters> targetParameters;
        std::map<std::string, PadDescriptor> padDescriptorsByName;
        std::map<int, TargetVariant> targetVariantsById;
        std::map<TargetRegisterType, TargetRegisterDescriptors> targetRegisterDescriptorsByType;
        std::map<TargetMemoryType, TargetMemoryDescriptor> targetMemoryDescriptorsByType;

        std::optional<ProgrammingSession> programmingSession;

        /**
         * Initiates the AVR8 instance from data extracted from the TDF.
         */
        void initFromTargetDescriptionFile();

        /**
         * Populates this->targetRegisterDescriptorsByType with registers extracted from the TDF, as well as general
         * purpose and other CPU registers.
         */
        void loadTargetRegisterDescriptors();

        void loadTargetMemoryDescriptors();

        /**
         * Extracts the ID from the target's memory.
         *
         * This function will cache the ID value and use the cached version for any subsequent calls.
         *
         * @return
         */
        TargetSignature getId() override;

        /**
         * Writes to FLASH memory (with any necessary erasing).
         *
         * @param startAddress
         * @param buffer
         */
        void writeFlashMemory(TargetMemoryAddress startAddress, const TargetMemoryBuffer& buffer);

        /**
         * Updates the debugWire enable (DWEN) fuse bit on the AVR target.
         *
         * @param enable
         *  True to enable the fuse, false to disable it.
         */
        void updateDwenFuseBit(bool enable);

        /**
         * Resolves the program memory section from a program memory address.
         *
         * @param address
         * @return
         */
        ProgramMemorySection getProgramMemorySectionFromAddress(TargetMemoryAddress address);
    };
}
