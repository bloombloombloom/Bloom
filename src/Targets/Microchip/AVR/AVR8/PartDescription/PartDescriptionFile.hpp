#pragma once

#include <QFile>
#include <QDomDocument>

#include "AddressSpace.hpp"
#include "MemorySegment.hpp"
#include "PropertyGroup.hpp"
#include "RegisterGroup.hpp"
#include "Module.hpp"
#include "Variant.hpp"
#include "Pinout.hpp"
#include "src/Targets/Microchip/AVR/TargetSignature.hpp"
#include "src/Targets/Microchip/AVR/AVR8/Family.hpp"

namespace Bloom::Targets::Microchip::Avr::Avr8Bit::PartDescription
{
    using Avr::TargetSignature;

    /**
     * An AVR8 part description file is an XML file that describes a particular AVR8 target.
     * All supported AVR8 targets come with a part description file.
     *
     * Part description files are part of the Bloom codebase.
     * For AVR8 part description files, see directory "src/Targets/Microchip/AVR/PartDescriptionFiles/AVR8".
     *
     * During the build process, all part description files are copied to the distribution directory, ready
     * to be shipped with the Bloom binary. Alongside these files is a JSON file, containing a mapping of AVR8 target
     * signatures to part description file paths. Bloom uses this mapping to find a particular part description
     * file, given a target signature. See directory "bin/Distribution/Resources/TargetPartDescriptions".
     * The copying of the part description files, and the generation of the JSON mapping, is done by a PHP script:
     * "build/scripts/CopyAvrPartFilesAndCreateMapping.php". This script is invoked via a custom command, at build time.
     *
     * All processing of AVR8 part description files is done in this class.
     */
    class PartDescriptionFile
    {
    private:
        QDomDocument xml;
        QDomElement deviceElement;
        mutable std::optional<std::map<std::string, PropertyGroup>> cachedPropertyGroupMapping;
        mutable std::optional<std::map<std::string, Module>> cachedModuleByNameMapping;
        mutable std::optional<std::map<std::string, Module>> cachedPeripheralModuleByNameMapping;
        mutable std::optional<std::map<std::string, Pinout>> cachedPinoutByNameMapping;

        /**`
         * AVR8 part description files include the target family name. This method returns a mapping of part
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
            };
        };

        void init(const QDomDocument& xml);
        void init(const QString& xmlFilePath);

        /**
         * Constructs an AddressSpace object from an XML element (in the form of a QDomElement), taken from
         * an AVR part description file.
         *
         * @param xmlElement
         * @return
         */
        AddressSpace generateAddressSpaceFromXml(const QDomElement& xmlElement) const;

        /**
         * Constructs a MemorySegment from an XML element (in the form of a QDomElement) taken from
         * an AVR part description file.
         *
         * @param xmlElement
         * @return
         */
        MemorySegment generateMemorySegmentFromXml(const QDomElement& xmlElement) const;
        RegisterGroup generateRegisterGroupFromXml(const QDomElement& xmlElement) const;

        Register generateRegisterFromXml(const QDomElement& xmlElement) const;

    public:
        /**
         * Will construct a PartDescription instance from the XML of a part description file, the path to which
         * is given via xmlFilePath.
         *
         * @param xmlFilePath
         */
        PartDescriptionFile(const QString& xmlFilePath) {
            this->init(xmlFilePath);
        }

        /**
         * Will construct a PartDescription instance from pre-loaded XML.
         *
         * @param xml
         */
        PartDescriptionFile(const QDomDocument& xml) {
            this->init(xml);
        }

        /**
         * Will resolve the part description file using the part description JSON mapping and a given target signature.
         *
         * @param targetSignatureHex
         * @param targetName
         */
        PartDescriptionFile(const std::string& targetSignatureHex, std::optional<std::string> targetName);

        /**
         * Loads the AVR8 target description JSON mapping file.
         *
         * @return
         */
        static QJsonObject getPartDescriptionMapping();

        std::string getTargetName() const;


        /**
         * Extracts the AVR8 target signature from the part description XML.
         *
         * @return
         */
        TargetSignature getTargetSignature() const;

        /**
         * Extracts all address spaces for the AVR8 target, from the part description XML.
         *
         * Will return a mapping of the extracted address spaces, mapped by id.
         *
         * @return
         */
        std::map<std::string, AddressSpace> getAddressSpacesMappedById() const;

        /**
         * Extracts the AVR8 target family from the part description XML.
         *
         * @return
         */
        Family getFamily() const;

        const std::map<std::string, PropertyGroup>& getPropertyGroupsMappedByName() const;
        const std::map<std::string, Module>& getModulesMappedByName() const;
        const std::map<std::string, Module>& getPeripheralModulesMappedByName() const;

        std::optional<MemorySegment> getFlashMemorySegment() const;
        std::optional<MemorySegment> getRamMemorySegment() const;
        std::optional<MemorySegment> getRegisterMemorySegment() const;
        std::optional<MemorySegment> getEepromMemorySegment() const;
        std::optional<MemorySegment> getFirstBootSectionMemorySegment() const;
        std::optional<RegisterGroup> getCpuRegisterGroup() const;
        std::optional<RegisterGroup> getEepromRegisterGroup() const;
        std::optional<Register> getStatusRegister() const;
        std::optional<Register> getStackPointerRegister() const;
        std::optional<Register> getStackPointerHighRegister() const;
        std::optional<Register> getStackPointerLowRegister() const;
        std::optional<Register> getOscillatorCalibrationRegister() const;
        std::optional<Register> getSpmcsrRegister() const;
        std::optional<Register> getEepromAddressRegister() const;
        std::optional<Register> getEepromDataRegister() const;
        std::optional<Register> getEepromControlRegister() const;
        std::vector<Variant> getVariants() const;
        const std::map<std::string, Pinout>& getPinoutsMappedByName() const;
    };
}
