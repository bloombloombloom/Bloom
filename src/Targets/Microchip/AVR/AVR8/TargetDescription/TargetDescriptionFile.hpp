#pragma once

#include "src/Targets/TargetDescription/TargetDescriptionFile.hpp"

#include "src/Targets/Microchip/AVR/TargetSignature.hpp"
#include "src/Targets/Microchip/AVR/AVR8/Family.hpp"

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

    public:
        /**
         * Will resolve the target description file using the target description JSON mapping and a given target signature.
         *
         * @param targetSignatureHex
         * @param targetName
         */
        TargetDescriptionFile(const TargetSignature& targetSignature, std::optional<std::string> targetName);

        /**
         * Loads the AVR8 target description JSON mapping file.
         *
         * @return
         */
        static QJsonObject getTargetDescriptionMapping();

        /**
         * Extracts the AVR8 target signature from the target description XML.
         *
         * @return
         */
        TargetSignature getTargetSignature() const;

        /**
         * Extracts the AVR8 target family from the target description XML.
         *
         * @return
         */
        Family getFamily() const;

        std::optional<Targets::TargetDescription::MemorySegment> getFlashMemorySegment() const;
        std::optional<Targets::TargetDescription::MemorySegment> getRamMemorySegment() const;
        std::optional<Targets::TargetDescription::MemorySegment> getRegisterMemorySegment() const;
        std::optional<Targets::TargetDescription::MemorySegment> getEepromMemorySegment() const;
        std::optional<Targets::TargetDescription::MemorySegment> getFirstBootSectionMemorySegment() const;
        std::optional<Targets::TargetDescription::RegisterGroup> getCpuRegisterGroup() const;
        std::optional<Targets::TargetDescription::RegisterGroup> getBootLoadRegisterGroup() const;
        std::optional<Targets::TargetDescription::RegisterGroup> getEepromRegisterGroup() const;
        std::optional<Targets::TargetDescription::Register> getStatusRegister() const;
        std::optional<Targets::TargetDescription::Register> getStackPointerRegister() const;
        std::optional<Targets::TargetDescription::Register> getStackPointerHighRegister() const;
        std::optional<Targets::TargetDescription::Register> getStackPointerLowRegister() const;
        std::optional<Targets::TargetDescription::Register> getOscillatorCalibrationRegister() const;
        std::optional<Targets::TargetDescription::Register> getSpmcsRegister() const;
        std::optional<Targets::TargetDescription::Register> getSpmcRegister() const;
        std::optional<Targets::TargetDescription::Register> getEepromAddressRegister() const;
        std::optional<Targets::TargetDescription::Register> getEepromAddressLowRegister() const;
        std::optional<Targets::TargetDescription::Register> getEepromAddressHighRegister() const;
        std::optional<Targets::TargetDescription::Register> getEepromDataRegister() const;
        std::optional<Targets::TargetDescription::Register> getEepromControlRegister() const;
    };
}
