#pragma once

#include <set>

#include "src/Targets/TargetDescription/TargetDescriptionFile.hpp"

#include "src/Targets/TargetVariant.hpp"
#include "src/Targets/TargetRegister.hpp"

#include "src/Targets/Microchip/AVR/TargetSignature.hpp"
#include "src/Targets/Microchip/AVR/IspParameters.hpp"
#include "src/Targets/Microchip/AVR/Fuse.hpp"

#include "src/Targets/Microchip/AVR/AVR8/Family.hpp"
#include "src/Targets/Microchip/AVR/AVR8/PhysicalInterface.hpp"
#include "src/Targets/Microchip/AVR/AVR8/TargetParameters.hpp"
#include "src/Targets/Microchip/AVR/AVR8/PadDescriptor.hpp"

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
         * Extracts the target's ISP parameters from the TDF.
         *
         * @return
         */
        [[nodiscard]] IspParameters getIspParameters() const;

        /**
         * Constructs a FuseBitDescriptor for the debugWire enable (DWEN) fuse bit, with information extracted from
         * the TDF.
         *
         * @return
         *  std::nullopt if the DWEN bit field could not be found in the TDF.
         */
        [[nodiscard]] std::optional<FuseBitsDescriptor> getDwenFuseBitsDescriptor() const;

        /**
         * Constructs a FuseBitDescriptor for the SPI enable (SPIEN) fuse bit, with information extracted from
         * the TDF.
         *
         * @return
         *  std::nullopt if the SPIEN bit field could not be found in the TDF.
         */
        [[nodiscard]] std::optional<FuseBitsDescriptor> getSpienFuseBitsDescriptor() const;

        /**
         * Returns a set of all supported physical interfaces for debugging.
         *
         * @return
         */
        [[nodiscard]] const auto& getSupportedPhysicalInterfaces() const {
            return this->supportedPhysicalInterfaces;
        }

        /**
         * Returns a mapping of all pad descriptors extracted from TDF, mapped by name.
         *
         * @return
         */
        [[nodiscard]] const auto& getPadDescriptorsMappedByName() const {
            return this->padDescriptorsByName;
        }

        /**
         * Returns a mapping of all target variants extracted from the TDF, mapped by ID.
         *
         * @return
         */
        [[nodiscard]] const auto& getVariantsMappedById() const {
            return this->targetVariantsById;
        }

        /**
         * Returns a mapping of all target register descriptors extracted from the TDF, by type.
         *
         * @return
         */
        [[nodiscard]] const auto& getRegisterDescriptorsMappedByType() const {
            return this->targetRegisterDescriptorsByType;
        }

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

        std::set<PhysicalInterface> supportedPhysicalInterfaces;

        std::map<std::string, PadDescriptor> padDescriptorsByName;
        std::map<int, TargetVariant> targetVariantsById;

        std::map<TargetRegisterType, TargetRegisterDescriptors> targetRegisterDescriptorsByType;

        /**
         * Populates this->supportedPhysicalInterfaces with physical interfaces defined in the TDF.
         */
        void loadSupportedPhysicalInterfaces();

        /**
         * Generates a collection of PadDescriptor objects from data in the TDF and populates this->padDescriptorsByName.
         */
        void loadPadDescriptors();

        /**
         * Loads all variants for the AVR8 target, from the TDF, and populates this->targetVariantsById.
         */
        void loadTargetVariants();

        /**
         * Loads all register descriptors from the TDF, and populates this->targetRegisterDescriptorsByType.
         */
        void loadTargetRegisterDescriptors();

        [[nodiscard]] std::optional<FuseBitsDescriptor> getFuseBitsDescriptorByName(
            const std::string& fuseBitName
        ) const;

        [[nodiscard]] std::optional<Targets::TargetDescription::AddressSpace> getProgramMemoryAddressSpace() const;
        [[nodiscard]] std::optional<Targets::TargetDescription::MemorySegment> getFlashApplicationMemorySegment(
            const Targets::TargetDescription::AddressSpace& programAddressSpace
        ) const;
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
    };
}
