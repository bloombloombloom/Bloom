#pragma once

#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <string>
#include <optional>
#include <functional>
#include <map>
#include <vector>

#include "AddressSpace.hpp"
#include "MemorySegment.hpp"
#include "PropertyGroup.hpp"
#include "RegisterGroup.hpp"
#include "Module.hpp"
#include "Variant.hpp"
#include "Pinout.hpp"
#include "Interface.hpp"

#include "src/Targets/TargetFamily.hpp"

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
    protected:
        std::map<std::string, std::string> deviceAttributesByName;
        std::map<std::string, AddressSpace> addressSpacesMappedById;
        std::map<std::string, PropertyGroup, std::less<void>> propertyGroupsMappedByKey;
        std::map<std::string, Module> modulesMappedByName;
        std::map<std::string, Module> peripheralModulesMappedByName;
        std::map<std::string, std::vector<RegisterGroup>> peripheralRegisterGroupsMappedByModuleRegisterGroupName;
        std::vector<Variant> variants;
        std::map<std::string, Pinout> pinoutsMappedByName;
        std::map<std::string, Interface> interfacesByName;

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

        /**
         * Constructs an AddressSpace object from an XML element.
         *
         * @param xmlElement
         * @return
         */
        static AddressSpace addressSpaceFromXml(const QDomElement& xmlElement);

        /**
         * Constructs a MemorySegment object from an XML element.
         *
         * @param xmlElement
         * @return
         */
        static MemorySegment memorySegmentFromXml(const QDomElement& xmlElement);

        /**
         * Constructs a RegisterGroup object from an XML element.
         *
         * @param xmlElement
         * @return
         */
        static RegisterGroup registerGroupFromXml(const QDomElement& xmlElement);

        /**
         * Constructs a Register object from an XML element.
         *
         * @param xmlElement
         * @return
         */
        static Register registerFromXml(const QDomElement& xmlElement);

        /**
         * Consturcts a BitField object from an XML element.
         *
         * @param xmlElement
         * @return
         */
        static BitField bitFieldFromXml(const QDomElement& xmlElement);

        /**
         * Fetches a device attribute value by name. Throws an exception if the attribute is not found.
         *
         * @param attributeName
         * @return
         */
        const std::string& deviceAttribute(const std::string& attributeName) const;

        /**
         * Extracts all address spaces and loads them into this->addressSpacesMappedById.
         */
        void loadAddressSpaces(const QDomDocument& document);

        /**
         * Extracts all modules and loads them into this->modulesMappedByName.
         */
        void loadModules(const QDomDocument& document);

        /**
         * Extracts all peripheral modules and loads them into this->peripheralModulesMappedByName.
         */
        void loadPeripheralModules(const QDomDocument& document);

        /**
         * Extracts all variants and loads them into this->variants.
         */
        void loadVariants(const QDomDocument& document);

        /**
         * Extracts all pinouts and loads them into this->pinoutsMappedByName.
         */
        void loadPinouts(const QDomDocument& document);

        /**
         * Extracts all interfaces and loads them into this->interfacesByName
         */
        void loadInterfaces(const QDomDocument& document);
    };
}
