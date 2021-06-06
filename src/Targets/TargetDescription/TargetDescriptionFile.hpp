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

namespace Bloom::Targets::TargetDescription
{
    /**
     * A target description file (TDF) is an XML file that describes a particular target. All supported targets come
     * with a target description file.
     *
     * Target description files are part of the Bloom codebase.
     * For target description files, see the directory "src/Targets/TargetDescriptionFiles/".
     *
     * During the build process, all target description files are copied to the distribution directory, ready
     * to be shipped with the Bloom binary.
     *
     * Processing of target description files is done in this class.
     *
     * This class may be extended to further reflect a TDF that is specific to a particular target, target architecture
     * or target family. For example, the Targets::Microchip::Avr::Avr8Bit::TargetDescription::TargetDescriptionFile
     * class inherits from this class, to represent TDFs for AVR8 targets. The derived class provides access to
     * additional data that is only found in AVR8 TDFs (such as AVR target signature, AVR Family, etc).
     */
    class TargetDescriptionFile
    {
    protected:
        QDomDocument xml;
        QDomElement deviceElement;

        void init(const QDomDocument& xml);
        void init(const QString& xmlFilePath);

    private:
        mutable std::optional<std::map<std::string, PropertyGroup>> cachedPropertyGroupMapping;
        mutable std::optional<std::map<std::string, Module>> cachedModuleByNameMapping;
        mutable std::optional<std::map<std::string, Module>> cachedPeripheralModuleByNameMapping;
        mutable std::optional<std::map<std::string, Pinout>> cachedPinoutByNameMapping;

        /**
         * Constructs an AddressSpace object from an XML element.
         *
         * @param xmlElement
         * @return
         */
        static AddressSpace generateAddressSpaceFromXml(const QDomElement& xmlElement);

        /**
         * Constructs a MemorySegment object from an XML element.
         *
         * @param xmlElement
         * @return
         */
        static MemorySegment generateMemorySegmentFromXml(const QDomElement& xmlElement);

        /**
         * Constructs a RegisterGroup object from an XML element.
         *
         * @param xmlElement
         * @return
         */
        static RegisterGroup generateRegisterGroupFromXml(const QDomElement& xmlElement);

        /**
         * Constructs a Register object from an XML element.
         *
         * @param xmlElement
         * @return
         */
        static Register generateRegisterFromXml(const QDomElement& xmlElement);

    public:
        TargetDescriptionFile() = default;

        /**
         * Will construct a TargetDescriptionFile instance from the XML of a target description file, the path to which
         * is given via xmlFilePath.
         *
         * @param xmlFilePath
         */
        explicit TargetDescriptionFile(const QString& xmlFilePath) {
            this->init(xmlFilePath);
        }

        /**
         * Will construct a TargetDescriptionFile instance from pre-loaded XML.
         *
         * @param xml
         */
        explicit TargetDescriptionFile(const QDomDocument& xml) {
            this->init(xml);
        }

        /**
         * Extracts target name.
         *
         * @return
         */
        std::string getTargetName() const;

        /**
         * Extracts all address spaces for the target.
         *
         * Will return a mapping of the extracted address spaces, mapped by id.
         *
         * @return
         */
        std::map<std::string, AddressSpace> getAddressSpacesMappedById() const;

        /**
         * Extracts all property groups and returns a mapping of them, with the property group name being the key.
         *
         * @return
         */
        const std::map<std::string, PropertyGroup>& getPropertyGroupsMappedByName() const;

        /**
         * Extracts all modules and returns a mapping of them, with the module name being the key.
         *
         * @return
         */
        const std::map<std::string, Module>& getModulesMappedByName() const;

        /**
         * Extracts all peripheral modules and returns a mapping of this, with the peripheral module name being
         * the key.
         *
         * @return
         */
        const std::map<std::string, Module>& getPeripheralModulesMappedByName() const;

        /**
         * Extracts all variants.
         *
         * @return
         */
        std::vector<Variant> getVariants() const;

        /**
         * Extracts all pinouts and returns a mapping of them, with the pinout name being the key.
         *
         * @return
         */
        const std::map<std::string, Pinout>& getPinoutsMappedByName() const;
    };
}
