#pragma once

#include <set>

#include "src/Targets/TargetDescription/TargetDescriptionFile.hpp"

#include "src/Targets/Microchip/AVR/TargetSignature.hpp"
#include "src/Targets/Microchip/AVR/AVR8/Family.hpp"
#include "src/Targets/Microchip/AVR/AVR8/PhysicalInterface.hpp"
#include "src/Targets/Microchip/AVR/AVR8/TargetParameters.hpp"
#include "src/Targets/Microchip/AVR/AVR8/PadDescriptor.hpp"
#include "src/Targets/TargetVariant.hpp"

namespace Bloom::Targets::Microchip::Avr::Avr8Bit::TargetDescription
{
    /**
     * Represents an AVR8 TDF. See the Targets::TargetDescription::TargetDescriptionFile close for more on TDFs.
     *
     * During the build process, we generate a JSON file containing a mapping of AVR8 target signatures to target
     * description file paths. Bloom uses this mapping to find a particular target description file, for AVR8 targets,
     * given a target signature. See directory "build/resources/TargetDescriptionFiles".
     * The generation of the JSON mapping, is done by a PHP script:
     * "build/scripts/CopyAvrPartFilesAndCreateMapping.php". This script is invoked via a custom command, at build time.
     *
     * For more information of TDFs, see src/Targets/TargetDescription/README.md
     */
    class TargetDescriptionFile: public Targets::TargetDescription::TargetDescriptionFile
    {
    private:
        /**`
         * AVR8 target description files include the target family name. This method returns a mapping of part
         * description family name strings to Family enum values.
         *
         * TODO: the difference in AVR8 family variations, like "tinyAVR" and "tinyAVR 2" may require attention.
         *
         * @return
         */
        static inline auto getFamilyNameToEnumMapping() {
            // All keys should be lower case.
            return std::map<std::string, Family> {
                {"megaavr", Family::MEGA},
                {"avr mega", Family::MEGA},
                {"avr xmega", Family::XMEGA},
                {"avr tiny", Family::TINY},
                {"tinyavr", Family::TINY},
                {"tinyavr 2", Family::TINY},
                {"avr da", Family::DA},
                {"avr db", Family::DB},
                {"avr dd", Family::DD},
            };
        };

        std::set<PhysicalInterface> supportedDebugPhysicalInterfaces;

        std::map<std::string, PadDescriptor> padDescriptorsByName;
        std::map<int, TargetVariant> targetVariantsById;

        /**
         * Populates this->supportedDebugPhysicalInterfaces with physical interfaces defined in the TDF.
         */
        void loadDebugPhysicalInterfaces();

        /**
         * Generates a collection of PadDescriptor objects from data in the TDF and populates this->padDescriptorsByName.
         */
        void loadPadDescriptors();

        /**
         * Loads all variants for the AVR8 target, from the TDF, and populates this->targetVariantsById.
         */
        void loadTargetVariants();

        [[nodiscard]] std::optional<Targets::TargetDescription::MemorySegment> getFlashMemorySegment() const;
        [[nodiscard]] std::optional<Targets::TargetDescription::MemorySegment> getRamMemorySegment() const;
        [[nodiscard]] std::optional<Targets::TargetDescription::MemorySegment> getIoMemorySegment() const;
        [[nodiscard]] std::optional<Targets::TargetDescription::MemorySegment> getRegisterMemorySegment() const;
        [[nodiscard]] std::optional<Targets::TargetDescription::MemorySegment> getEepromMemorySegment() const;
        [[nodiscard]] std::optional<Targets::TargetDescription::MemorySegment> getFirstBootSectionMemorySegment() const;
        [[nodiscard]] std::optional<Targets::TargetDescription::MemorySegment> getSignatureMemorySegment() const;
        [[nodiscard]] std::optional<Targets::TargetDescription::MemorySegment> getFuseMemorySegment() const;
        [[nodiscard]] std::optional<Targets::TargetDescription::MemorySegment> getLockbitsMemorySegment() const;
        [[nodiscard]] std::optional<Targets::TargetDescription::RegisterGroup> getCpuRegisterGroup() const;
        [[nodiscard]] std::optional<Targets::TargetDescription::RegisterGroup> getBootLoadRegisterGroup() const;
        [[nodiscard]] std::optional<Targets::TargetDescription::RegisterGroup> getEepromRegisterGroup() const;

        [[nodiscard]] std::optional<Targets::TargetDescription::Register> getStatusRegister() const;
        [[nodiscard]] std::optional<Targets::TargetDescription::Register> getStackPointerRegister() const;
        [[nodiscard]] std::optional<Targets::TargetDescription::Register> getStackPointerHighRegister() const;
        [[nodiscard]] std::optional<Targets::TargetDescription::Register> getStackPointerLowRegister() const;
        [[nodiscard]] std::optional<Targets::TargetDescription::Register> getOscillatorCalibrationRegister() const;
        [[nodiscard]] std::optional<Targets::TargetDescription::Register> getSpmcsRegister() const;
        [[nodiscard]] std::optional<Targets::TargetDescription::Register> getSpmcRegister() const;
        [[nodiscard]] std::optional<Targets::TargetDescription::Register> getEepromAddressRegister() const;
        [[nodiscard]] std::optional<Targets::TargetDescription::Register> getEepromAddressLowRegister() const;
        [[nodiscard]] std::optional<Targets::TargetDescription::Register> getEepromAddressHighRegister() const;
        [[nodiscard]] std::optional<Targets::TargetDescription::Register> getEepromDataRegister() const;
        [[nodiscard]] std::optional<Targets::TargetDescription::Register> getEepromControlRegister() const;

        /**
         * Loads target parameters that are specific to debugWire and mega JTAG sessions.
         *
         * @param targetParameters
         */
        virtual void loadDebugWireAndJtagTargetParameters(TargetParameters& targetParameters) const;

        /**
         * Loads target parameters that are specific to PDI sessions.
         *
         * @param targetParameters
         */
        virtual void loadPdiTargetParameters(TargetParameters& targetParameters) const;

        /**
         * Loads target parameters that are specific to UPDI sessions.
         *
         * @param targetParameters
         */
        virtual void loadUpdiTargetParameters(TargetParameters& targetParameters) const;

    public:
        /**
         * Will resolve the target description file using the target description JSON mapping and a given target signature.
         *
         * @param targetSignatureHex
         * @param targetName
         */
        TargetDescriptionFile(const TargetSignature& targetSignature, std::optional<std::string> targetName);

        /**
         * Extends TDF initialisation to include the loading of physical interfaces for debugging AVR8 targets, among
         * other things.
         *
         * @param xml
         */
        void init(const QDomDocument& xml) override;

        /**
         * Loads the AVR8 target description JSON mapping file.
         *
         * @return
         */
        static QJsonObject getTargetDescriptionMapping();

        /**
         * Extracts the AVR8 target signature from the TDF.
         *
         * @return
         */
        [[nodiscard]] TargetSignature getTargetSignature() const;

        /**
         * Extracts the AVR8 target family from the TDF.
         *
         * @return
         */
        [[nodiscard]] Family getFamily() const;

        /**
         * Constructs an instance of TargetParameters, for the AVR8 target, with data from the TDF.
         *
         * @return
         */
        [[nodiscard]] TargetParameters getTargetParameters() const;

        /**
         * Returns a set of all supported physical interfaces for debugging.
         *
         * @return
         */
        [[nodiscard]] const auto& getSupportedDebugPhysicalInterfaces() const {
            return this->supportedDebugPhysicalInterfaces;
        };

        [[nodiscard]] const auto& getPadDescriptorsMappedByName() const {
            return this->padDescriptorsByName;
        };

        [[nodiscard]] const auto& getVariantsMappedById() const {
            return this->targetVariantsById;
        };
    };
}
