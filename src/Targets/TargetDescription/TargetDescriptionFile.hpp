#pragma once

#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <string>
#include <optional>
#include <functional>
#include <map>
#include <vector>

#include "PropertyGroup.hpp"
#include "AddressSpace.hpp"
#include "MemorySegment.hpp"
#include "MemorySegmentSection.hpp"
#include "PhysicalInterface.hpp"
#include "Module.hpp"
#include "RegisterGroup.hpp"
#include "Register.hpp"
#include "RegisterGroupReference.hpp"
#include "Peripheral.hpp"
#include "RegisterGroupInstance.hpp"
#include "Signal.hpp"
#include "Pinout.hpp"
#include "Variant.hpp"

#include "src/Targets/TargetFamily.hpp"
#include "src/Targets/TargetPhysicalInterface.hpp"

#include GENERATED_TDF_MAPPING_PATH

namespace Targets::TargetDescription
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
     *
     * For more information of TDFs, see src/Targets/TargetDescription/README.md
     */
    class TargetDescriptionFile
    {
    public:
        /**
         * Returns a mapping of target configuration values to instances of the GeneratedMapping::BriefTargetDescriptor
         * struct.
         *
         * The mapping is generated pre-build.
         *
         * The GeneratedMapping::BriefTargetDescriptor struct holds some brief info about a particular target, such as
         * target name, family and TDF path. See the GeneratedMapping.hpp.in template for more.
         *
         * @return
         */
        static const std::map<std::string, GeneratedMapping::BriefTargetDescriptor>& mapping();

        /**
         * Will construct a TargetDescriptionFile instance from the XML of a target description file, the path to which
         * is given via xmlFilePath.
         *
         * @param xmlFilePath
         */
        explicit TargetDescriptionFile(const std::string& xmlFilePath);

        /**
         * Will construct a TargetDescriptionFile instance from pre-loaded XML.
         *
         * @param xml
         */
        explicit TargetDescriptionFile(const QDomDocument& xml);

        /**
         * Returns the target name extracted from the TDF.
         *
         * @return
         */
        [[nodiscard]] const std::string& getTargetName() const;

        /**
         * Returns the target family extracted from the TDF.
         *
         * @return
         */
        [[nodiscard]] TargetFamily getFamily() const;

        [[nodiscard]] std::optional<std::reference_wrapper<const PropertyGroup>> tryGetPropertyGroup(
            std::string_view keyStr
        ) const;
        [[nodiscard]] const PropertyGroup& getPropertyGroup(std::string_view keyStr) const;

        [[nodiscard]] std::optional<std::reference_wrapper<const AddressSpace>> tryGetAddressSpace(
            std::string_view key
        ) const;
        [[nodiscard]] const AddressSpace& getAddressSpace(std::string_view key) const;

        [[nodiscard]] std::set<Targets::TargetPhysicalInterface> getPhysicalInterfaces() const;

        [[nodiscard]] std::optional<std::reference_wrapper<const Module>> tryGetModule(
            std::string_view key
        ) const;
        [[nodiscard]] const Module& getModule(std::string_view key) const;

        [[nodiscard]] std::optional<std::reference_wrapper<const Peripheral>> tryGetPeripheral(
            std::string_view key
        ) const;
        [[nodiscard]] const Peripheral& getPeripheral(std::string_view key) const;

    protected:
        std::map<std::string, std::string> deviceAttributesByName;
        std::map<std::string, PropertyGroup, std::less<void>> propertyGroupsByKey;
        std::map<std::string, AddressSpace, std::less<void>> addressSpacesByKey;
        std::vector<PhysicalInterface> physicalInterfaces;
        std::map<std::string, Module, std::less<void>> modulesByKey;
        std::map<std::string, Peripheral, std::less<void>> peripheralsByKey;
        std::map<std::string, std::vector<RegisterGroup>> peripheralRegisterGroupsMappedByModuleRegisterGroupName;
        std::vector<Variant> variants;
        std::map<std::string, Pinout> pinoutsMappedByName;

        TargetDescriptionFile() = default;
        virtual ~TargetDescriptionFile() = default;

        TargetDescriptionFile(const TargetDescriptionFile& other) = default;
        TargetDescriptionFile(TargetDescriptionFile&& other) = default;

        TargetDescriptionFile& operator = (const TargetDescriptionFile& other) = default;
        TargetDescriptionFile& operator = (TargetDescriptionFile&& other) = default;

        void init(const std::string& xmlFilePath);
        void init(const QDomDocument& document);

        static std::optional<std::string> tryGetAttribute(const QDomElement& element, const QString& attributeName);
        static std::string getAttribute(const QDomElement& element, const QString& attributeName);

        static PropertyGroup propertyGroupFromXml(const QDomElement& xmlElement);
        static Property propertyFromXml(const QDomElement& xmlElement);
        static AddressSpace addressSpaceFromXml(const QDomElement& xmlElement);
        static MemorySegment memorySegmentFromXml(const QDomElement& xmlElement);
        static MemorySegmentSection memorySegmentSectionFromXml(const QDomElement& xmlElement);
        static PhysicalInterface physicalInterfaceFromXml(const QDomElement& xmlElement);
        static Module moduleFromXml(const QDomElement& xmlElement);
        static RegisterGroup registerGroupFromXml(const QDomElement& xmlElement);
        static RegisterGroupReference registerGroupReferenceFromXml(const QDomElement& xmlElement);
        static Register registerFromXml(const QDomElement& xmlElement);
        static BitField bitFieldFromXml(const QDomElement& xmlElement);
        static Peripheral peripheralFromXml(const QDomElement& xmlElement);
        static RegisterGroupInstance registerGroupInstanceFromXml(const QDomElement& xmlElement);
        static Signal signalFromXml(const QDomElement& xmlElement);

        /**
         * Fetches a device attribute value by name. Throws an exception if the attribute is not found.
         *
         * @param attributeName
         * @return
         */
        const std::string& deviceAttribute(const std::string& attributeName) const;

        /**
         * Extracts all variants and loads them into this->variants.
         */
        void loadVariants(const QDomDocument& document);

        /**
         * Extracts all pinouts and loads them into this->pinoutsMappedByName.
         */
        void loadPinouts(const QDomDocument& document);
    };
}
