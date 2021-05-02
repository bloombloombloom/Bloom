#pragma once

#include <cstdint>
#include <queue>

#include "src/DebugToolDrivers/TargetInterfaces/Microchip/AVR/AVR8/Avr8Interface.hpp"
#include "src/Targets/Microchip/AVR/Target.hpp"
#include "src/Targets/TargetRegister.hpp"
#include "src/DebugToolDrivers/DebugTool.hpp"
#include "src/ApplicationConfig.hpp"
#include "src/Exceptions/Exception.hpp"

#include "TargetParameters.hpp"
#include "Family.hpp"
#include "PadDescriptor.hpp"

// Part Description
#include "PartDescription/PartDescriptionFile.hpp"

namespace Bloom::Targets::Microchip::Avr::Avr8Bit
{
    using namespace Exceptions;
    using namespace PartDescription;

    using PartDescription::PartDescriptionFile;
    using DebugToolDrivers::TargetInterfaces::Microchip::Avr::Avr8::Avr8Interface;
    using Targets::TargetRegisterMap;

    class Avr8: public Target
    {
    protected:
        Avr8Interface* avr8Interface;
        std::string name = "";
        std::optional<Family> family;
        std::optional<PartDescriptionFile> partDescription;
        std::optional<TargetParameters> targetParameters;
        std::map<std::string, PadDescriptor> padDescriptorsByName;
        std::map<int, TargetVariant> targetVariantsById;

        /**
         * Extracts the ID from the target's memory.
         *
         * This function will cache the ID value and use the cached version for any subsequent calls.
         *
         * @return
         */
        TargetSignature getId() override;

        /**
         * Extracts the AVR8 target parameters from the loaded part description file.
         *
         * @return
         */
        virtual TargetParameters& getTargetParameters();

        /**
         * Generates a collection of PadDescriptor object from data in the loaded part description file and
         * populates this->padDescriptorsByName.
         */
        virtual void loadPadDescriptors();

        /**
         * Extracts target variant information from the loaded part description file and generates a collection
         * of TargetVariant objects.
         *
         * @return
         */
        virtual std::vector<TargetVariant> generateVariantsFromPartDescription();

        /**
         * Populates this->targetVariantsById using this->generateVariantsFromPartDescription() and data from
         * this->padDescriptorsByName.
         */
        virtual void loadTargetVariants();

        void loadPartDescription();

    public:
        explicit Avr8() = default;
        Avr8(const std::string& name, const TargetSignature& signature): name(name) {
            this->id = signature;
        };

        /*
         * The functions below implement the Target interface for AVR8 targets.
         *
         * See the Bloom::Targets::Target interface class for documentation on the expected behaviour of
         * each function.
         */

        void preActivationConfigure(const TargetConfig& targetConfig) override;
        void postActivationConfigure() override;
        virtual void postPromotionConfigure() override;

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
        virtual std::unique_ptr<Targets::Target> promote() override;

        std::string getName() const override {
            return this->name;
        }

        virtual TargetDescriptor getDescriptor() override;

        void run() override;
        void stop() override;
        void step() override;
        void reset() override;

        void setBreakpoint(std::uint32_t address) override;
        void removeBreakpoint(std::uint32_t address) override;
        void clearAllBreakpoints() override;

        virtual TargetRegisters readGeneralPurposeRegisters(std::set<std::size_t> registerIds) override;
        virtual void writeRegisters(const TargetRegisters& registers) override;
        virtual TargetRegisters readRegisters(const TargetRegisterDescriptors& descriptors) override;

        virtual TargetMemoryBuffer readMemory(
            TargetMemoryType memoryType,
            std::uint32_t startAddress,
            std::uint32_t bytes
        ) override;
        virtual void writeMemory(
            TargetMemoryType memoryType,
            std::uint32_t startAddress,
            const TargetMemoryBuffer& buffer
        ) override;

        virtual TargetState getState() override;

        virtual std::uint32_t getProgramCounter() override;
        virtual TargetRegister getProgramCounterRegister() override;
        virtual void setProgramCounter(std::uint32_t programCounter) override;

        virtual TargetRegister getStackPointerRegister() override;
        virtual TargetRegister getStatusRegister() override;

        virtual std::map<int, TargetPinState> getPinStates(int variantId) override;
        virtual void setPinState(
            int variantId,
            const TargetPinDescriptor& pinDescriptor,
            const TargetPinState& state
        ) override;

        virtual bool memoryAddressRangeClashesWithIoPortRegisters(
            TargetMemoryType memoryType,
            std::uint32_t startAddress,
            std::uint32_t endAddress
        ) override;
    };
}
