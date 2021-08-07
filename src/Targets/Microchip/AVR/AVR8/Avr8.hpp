#pragma once

#include <cstdint>
#include <queue>
#include <utility>

#include "src/DebugToolDrivers/TargetInterfaces/Microchip/AVR/AVR8/Avr8Interface.hpp"
#include "src/Targets/Microchip/AVR/Target.hpp"
#include "src/Targets/TargetRegister.hpp"
#include "src/DebugToolDrivers/DebugTool.hpp"
#include "src/ApplicationConfig.hpp"
#include "src/Exceptions/Exception.hpp"

#include "TargetParameters.hpp"
#include "Family.hpp"
#include "PadDescriptor.hpp"

#include "TargetDescription/TargetDescriptionFile.hpp"

namespace Bloom::Targets::Microchip::Avr::Avr8Bit
{
    class Avr8: public Target
    {
    protected:
        DebugToolDrivers::TargetInterfaces::Microchip::Avr::Avr8::Avr8Interface* avr8Interface = nullptr;
        std::string name;
        std::optional<Family> family;
        std::optional<TargetDescription::TargetDescriptionFile> targetDescriptionFile;
        std::optional<TargetParameters> targetParameters;
        std::map<std::string, PadDescriptor> padDescriptorsByName;
        std::map<int, TargetVariant> targetVariantsById;
        std::map<TargetRegisterType, std::vector<TargetRegisterDescriptor>> targetRegisterDescriptorsByType;

        /**
         * Resolves the appropriate TDF for the AVR8 target and populates this->targetDescriptionFile.
         */
        void loadTargetDescriptionFile();

        /**
         * Initiates the AVR8 instance from data extracted from the TDF.
         */
        void initFromTargetDescriptionFile();

        /**
         * Populates this->targetRegisterDescriptorsByType with registers extracted from the TDF, as well as general
         * purpose and other CPU registers.
         */
        void loadTargetRegisterDescriptors();

        /**
         * Extracts the ID from the target's memory.
         *
         * This function will cache the ID value and use the cached version for any subsequent calls.
         *
         * @return
         */
        TargetSignature getId() override;

    public:
        explicit Avr8() = default;
        Avr8(std::string name, const TargetSignature& signature): name(std::move(name)) {
            this->id = signature;
            this->loadTargetDescriptionFile();
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
            return debugTool->getAvr8Interface() != nullptr;
        }

        void setDebugTool(DebugTool* debugTool) override {
            this->avr8Interface = debugTool->getAvr8Interface();
        };

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

        void setBreakpoint(std::uint32_t address) override;
        void removeBreakpoint(std::uint32_t address) override;
        void clearAllBreakpoints() override;

        void writeRegisters(TargetRegisters registers) override;
        TargetRegisters readRegisters(TargetRegisterDescriptors descriptors) override;

        TargetMemoryBuffer readMemory(
            TargetMemoryType memoryType,
            std::uint32_t startAddress,
            std::uint32_t bytes
        ) override;
        void writeMemory(
            TargetMemoryType memoryType,
            std::uint32_t startAddress,
            const TargetMemoryBuffer& buffer
        ) override;

        TargetState getState() override;

        std::uint32_t getProgramCounter() override;
        TargetRegister getProgramCounterRegister();
        void setProgramCounter(std::uint32_t programCounter) override;

        std::map<int, TargetPinState> getPinStates(int variantId) override;
        void setPinState(
            int variantId,
            const TargetPinDescriptor& pinDescriptor,
            const TargetPinState& state
        ) override;

        bool memoryAddressRangeClashesWithIoPortRegisters(
            TargetMemoryType memoryType,
            std::uint32_t startAddress,
            std::uint32_t endAddress
        ) override;
    };
}
