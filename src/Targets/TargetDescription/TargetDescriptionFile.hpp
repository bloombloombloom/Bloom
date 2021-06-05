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
         * Constructs an AddressSpace object from an XML element (in the form of a QDomElement), taken from a target
         * description file.
         *
         * @param xmlElement
         * @return
         */
        AddressSpace generateAddressSpaceFromXml(const QDomElement& xmlElement) const;

        /**
         * Constructs a MemorySegment from an XML element (in the form of a QDomElement) taken from a target
         * description file.
         *
         * @param xmlElement
         * @return
         */
        MemorySegment generateMemorySegmentFromXml(const QDomElement& xmlElement) const;
        RegisterGroup generateRegisterGroupFromXml(const QDomElement& xmlElement) const;

        Register generateRegisterFromXml(const QDomElement& xmlElement) const;

    public:
        TargetDescriptionFile() = default;

        /**
         * Will construct a TargetDescriptionFile instance from the XML of a target description file, the path to which
         * is given via xmlFilePath.
         *
         * @param xmlFilePath
         */
        TargetDescriptionFile(const QString& xmlFilePath) {
            this->init(xmlFilePath);
        }

        /**
         * Will construct a TargetDescriptionFile instance from pre-loaded XML.
         *
         * @param xml
         */
        TargetDescriptionFile(const QDomDocument& xml) {
            this->init(xml);
        }

        std::string getTargetName() const;

        /**
         * Extracts all address spaces for the AVR8 target, from the target description XML.
         *
         * Will return a mapping of the extracted address spaces, mapped by id.
         *
         * @return
         */
        std::map<std::string, AddressSpace> getAddressSpacesMappedById() const;

        const std::map<std::string, PropertyGroup>& getPropertyGroupsMappedByName() const;
        const std::map<std::string, Module>& getModulesMappedByName() const;
        const std::map<std::string, Module>& getPeripheralModulesMappedByName() const;

        std::optional<MemorySegment> getFlashMemorySegment() const;
        std::optional<MemorySegment> getRamMemorySegment() const;
        std::optional<MemorySegment> getRegisterMemorySegment() const;
        std::optional<MemorySegment> getEepromMemorySegment() const;
        std::optional<MemorySegment> getFirstBootSectionMemorySegment() const;
        std::optional<RegisterGroup> getCpuRegisterGroup() const;
        std::optional<RegisterGroup> getBootLoadRegisterGroup() const;
        std::optional<RegisterGroup> getEepromRegisterGroup() const;
        std::optional<Register> getStatusRegister() const;
        std::optional<Register> getStackPointerRegister() const;
        std::optional<Register> getStackPointerHighRegister() const;
        std::optional<Register> getStackPointerLowRegister() const;
        std::optional<Register> getOscillatorCalibrationRegister() const;
        std::optional<Register> getSpmcsRegister() const;
        std::optional<Register> getSpmcRegister() const;
        std::optional<Register> getEepromAddressRegister() const;
        std::optional<Register> getEepromAddressLowRegister() const;
        std::optional<Register> getEepromAddressHighRegister() const;
        std::optional<Register> getEepromDataRegister() const;
        std::optional<Register> getEepromControlRegister() const;
        std::vector<Variant> getVariants() const;
        const std::map<std::string, Pinout>& getPinoutsMappedByName() const;
    };
}
