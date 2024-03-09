#include "TargetDescriptionFile.hpp"

#include <QJsonDocument>
#include <QJsonArray>

#include "src/Services/PathService.hpp"
#include "src/Services/StringService.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetPinoutDescriptor.hpp"
#include "src/Helpers/BiMap.hpp"
#include "src/Logger/Logger.hpp"

#include "Exceptions/TargetDescriptionParsingFailureException.hpp"
#include "Exceptions/InvalidTargetDescriptionDataException.hpp"

namespace Targets::TargetDescription
{
    using namespace Exceptions;
    using namespace ::Exceptions;

    using Services::StringService;

    TargetDescriptionFile::TargetDescriptionFile(const std::string& xmlFilePath) {
        this->init(xmlFilePath);
    }

    TargetDescriptionFile::TargetDescriptionFile(const QDomDocument& xml) {
        this->init(xml);
    }

    const std::string& TargetDescriptionFile::getTargetName() const {
        return this->deviceAttribute("name");
    }

    TargetFamily TargetDescriptionFile::getFamily() const {
        const auto& family = this->deviceAttribute("family");

        if (family == "AVR8") {
            return TargetFamily::AVR_8;
        }

        if (family == "RISCV") {
            return TargetFamily::RISC_V;
        }

        throw InvalidTargetDescriptionDataException("Failed to resolve target family - invalid family name");
    }

    std::optional<std::reference_wrapper<const PropertyGroup>> TargetDescriptionFile::tryGetPropertyGroup(
        std::string_view keyStr
    ) const {
        const auto keys = StringService::split(keyStr, '.');

        const auto firstSubgroupIt = this->propertyGroupsByKey.find(*keys.begin());
        return firstSubgroupIt != this->propertyGroupsByKey.end()
            ? keys.size() > 1
                ? firstSubgroupIt->second.tryGetSubgroup(keys | std::ranges::views::drop(1))
                : std::optional(std::cref(firstSubgroupIt->second))
            : std::nullopt;
    }

    const PropertyGroup& TargetDescriptionFile::getPropertyGroup(std::string_view keyStr) const {
        const auto propertyGroup = this->tryGetPropertyGroup(keyStr);

        if (!propertyGroup.has_value()) {
            throw InvalidTargetDescriptionDataException(
                "Failed to get property group \"" + std::string(keyStr) + "\" from TDF - property group not found"
            );
        }

        return propertyGroup->get();
    }

    std::optional<std::reference_wrapper<const AddressSpace>> TargetDescriptionFile::tryGetAddressSpace(
        std::string_view key
    ) const {
        const auto addressSpaceIt = this->addressSpacesByKey.find(key);
        return addressSpaceIt != this->addressSpacesByKey.end()
            ? std::optional(std::cref(addressSpaceIt->second))
            : std::nullopt;
    }

    const AddressSpace& TargetDescriptionFile::getAddressSpace(std::string_view key) const {
        const auto addressSpace = this->tryGetAddressSpace(key);

        if (!addressSpace.has_value()) {
            throw InvalidTargetDescriptionDataException(
                "Failed to get address space \"" + std::string(key) + "\" from TDF - address space not found"
            );
        }

        return addressSpace->get();
    }

    std::set<TargetPhysicalInterface> TargetDescriptionFile::getPhysicalInterfaces() const {
        static const auto physicalInterfacesByName = BiMap<std::string, TargetPhysicalInterface>({
            {"updi", TargetPhysicalInterface::UPDI},
            {"debugwire", TargetPhysicalInterface::DEBUG_WIRE},
            {"jtag", TargetPhysicalInterface::JTAG},
            {"pdi", TargetPhysicalInterface::PDI},
            {"isp", TargetPhysicalInterface::ISP},
        });

        auto output = std::set<TargetPhysicalInterface>();

        for (const auto& physicalInterface : this->physicalInterfaces) {
            const auto interface = physicalInterfacesByName.valueAt(
                StringService::asciiToLower(physicalInterface.name)
            );

            if (interface.has_value()) {
                output.insert(*interface);
            }
        }

        return output;
    }

    std::optional<std::reference_wrapper<const Module>> TargetDescriptionFile::tryGetModule(
        std::string_view key
    ) const {
        const auto moduleIt = this->modulesByKey.find(key);
        return moduleIt != this->modulesByKey.end()
            ? std::optional(std::cref(moduleIt->second))
            : std::nullopt;
    }

    const Module& TargetDescriptionFile::getModule(std::string_view key) const {
        const auto module = this->tryGetModule(key);

        if (!module.has_value()) {
            throw InvalidTargetDescriptionDataException(
                "Failed to get module \"" + std::string(key) + "\" from TDF - module not found"
            );
        }

        return module->get();
    }

    std::optional<std::reference_wrapper<const Peripheral>> TargetDescriptionFile::tryGetPeripheral(
        std::string_view key
    ) const {
        const auto peripheralIt = this->peripheralsByKey.find(key);
        return peripheralIt != this->peripheralsByKey.end()
            ? std::optional(std::cref(peripheralIt->second))
            : std::nullopt;
    }

    const Peripheral& TargetDescriptionFile::getPeripheral(std::string_view key) const {
        const auto peripheral = this->tryGetPeripheral(key);

        if (!peripheral.has_value()) {
            throw InvalidTargetDescriptionDataException(
                "Failed to get peripheral \"" + std::string(key) + "\" from TDF - peripheral not found"
            );
        }

        return peripheral->get();
    }

    void TargetDescriptionFile::init(const std::string& xmlFilePath) {
        auto file = QFile(QString::fromStdString(xmlFilePath));
        if (!file.exists()) {
            throw InternalFatalErrorException("Failed to load target description file - file not found");
        }

        file.open(QIODevice::ReadOnly);
        auto document = QDomDocument();
        if (!document.setContent(file.readAll())) {
            throw TargetDescriptionParsingFailureException();
        }

        this->init(document);
    }

    void TargetDescriptionFile::init(const QDomDocument& document) {
        const auto deviceElement = document.documentElement();
        if (deviceElement.nodeName() != "device") {
            throw TargetDescriptionParsingFailureException("Root \"device\" element not found.");
        }

        const auto deviceAttributes = deviceElement.attributes();
        for (auto i = 0; i < deviceAttributes.length(); ++i) {
            const auto deviceAttribute = deviceAttributes.item(i);
            this->deviceAttributesByName.insert(
                std::pair(
                    deviceAttribute.nodeName().toStdString(),
                    deviceAttribute.nodeValue().toStdString()
                )
            );
        }

        for (
            auto element = deviceElement.firstChildElement("property-groups").firstChildElement("property-group");
            !element.isNull();
            element = element.nextSiblingElement("property-group")
        ) {
            auto propertyGroup = TargetDescriptionFile::propertyGroupFromXml(element);
            this->propertyGroupsByKey.insert(
                std::pair(propertyGroup.key, std::move(propertyGroup))
            );
        }

        for (
            auto element = deviceElement.firstChildElement("address-spaces").firstChildElement("address-space");
            !element.isNull();
            element = element.nextSiblingElement("address-space")
        ) {
            auto addressSpace = TargetDescriptionFile::addressSpaceFromXml(element);
            this->addressSpacesByKey.insert(
                std::pair(addressSpace.key, std::move(addressSpace))
            );
        }

        for (
            auto element = deviceElement.firstChildElement("physical-interfaces")
                .firstChildElement("physical-interface");
            !element.isNull();
            element = element.nextSiblingElement("physical-interface")
        ) {
            this->physicalInterfaces.emplace_back(
                TargetDescriptionFile::physicalInterfaceFromXml(element)
            );
        }

        for (
            auto element = deviceElement.firstChildElement("modules").firstChildElement("module");
            !element.isNull();
            element = element.nextSiblingElement("module")
        ) {
            auto module = TargetDescriptionFile::moduleFromXml(element);
            this->modulesByKey.insert(
                std::pair(module.key, std::move(module))
            );
        }

        for (
            auto element = deviceElement.firstChildElement("peripherals").firstChildElement("peripheral");
            !element.isNull();
            element = element.nextSiblingElement("peripheral")
        ) {
            auto peripheral = TargetDescriptionFile::peripheralFromXml(element);
            this->peripheralsByKey.insert(
                std::pair(peripheral.key, std::move(peripheral))
            );
        }

        for (
            auto element = deviceElement.firstChildElement("pinouts").firstChildElement("pinout");
            !element.isNull();
            element = element.nextSiblingElement("pinout")
        ) {
            auto pinout = TargetDescriptionFile::pinoutFromXml(element);
            this->pinoutsByKey.insert(
                std::pair(pinout.key, std::move(pinout))
            );
        }

        for (
            auto element = deviceElement.firstChildElement("variants").firstChildElement("variant");
            !element.isNull();
            element = element.nextSiblingElement("variant")
        ) {
            this->variants.emplace_back(TargetDescriptionFile::variantFromXml(element));
        }
    }

    const std::string& TargetDescriptionFile::deviceAttribute(const std::string& attributeName) const {
        const auto attributeIt = this->deviceAttributesByName.find(attributeName);

        if (attributeIt == this->deviceAttributesByName.end()) {
            throw InvalidTargetDescriptionDataException("Missing target device attribute (\"" + attributeName + "\")");
        }

        return attributeIt->second;
    }

    std::optional<std::string> TargetDescriptionFile::tryGetAttribute(
        const QDomElement& element,
        const QString& attributeName
    ) {
        return element.hasAttribute(attributeName)
            ? std::optional(element.attribute(attributeName).toStdString())
            : std::nullopt;
    }

    std::string TargetDescriptionFile::getAttribute(const QDomElement& element, const QString& attributeName) {
        const auto attribute = TargetDescriptionFile::tryGetAttribute(element, attributeName);

        if (!attribute.has_value()) {
            throw InvalidTargetDescriptionDataException(
                "Failed to fetch attribute from TDF element \"" + element.nodeName().toStdString()
                    + "\" - attribute \"" + attributeName.toStdString() + "\" not found"
            );
        }

        return *attribute;
    }

    PropertyGroup TargetDescriptionFile::propertyGroupFromXml(const QDomElement& xmlElement) {
        auto output = PropertyGroup(TargetDescriptionFile::getAttribute(xmlElement, "key"), {}, {});

        for (
            auto element = xmlElement.firstChildElement("property");
            !element.isNull();
            element = element.nextSiblingElement("property")
        ) {
            auto property = TargetDescriptionFile::propertyFromXml(element);
            output.propertiesByKey.insert(std::pair(property.key, std::move(property)));
        }

        for (
            auto element = xmlElement.firstChildElement("property-group");
            !element.isNull();
            element = element.nextSiblingElement("property-group")
        ) {
            auto subgroup = TargetDescriptionFile::propertyGroupFromXml(element);
            output.subgroupsByKey.insert(std::pair(subgroup.key, std::move(subgroup)));
        }

        return output;
    }

    Property TargetDescriptionFile::propertyFromXml(const QDomElement& xmlElement) {
        return Property(
            TargetDescriptionFile::getAttribute(xmlElement, "key"),
            TargetDescriptionFile::getAttribute(xmlElement, "value")
        );
    }

    AddressSpace TargetDescriptionFile::addressSpaceFromXml(const QDomElement& xmlElement) {
        static const auto endiannessByName = BiMap<std::string, TargetMemoryEndianness>({
            {"big", TargetMemoryEndianness::BIG},
            {"little", TargetMemoryEndianness::LITTLE},
        });

        const auto endiannessName = TargetDescriptionFile::tryGetAttribute(xmlElement, "endianness");

        auto endianness = std::optional<TargetMemoryEndianness>();
        if (endiannessName.has_value()) {
            endianness = endiannessByName.valueAt(*endiannessName);

            if (!endianness.has_value()) {
                throw InvalidTargetDescriptionDataException(
                    "Failed to extract address space from TDF - invalid endianness name \"" + *endiannessName + "\""
                );
            }
        }

        auto output = AddressSpace(
            TargetDescriptionFile::getAttribute(xmlElement, "key"),
            StringService::toUint32(TargetDescriptionFile::getAttribute(xmlElement, "start")),
            StringService::toUint32(TargetDescriptionFile::getAttribute(xmlElement, "size")),
            endianness,
            {}
        );

        for (
            auto element = xmlElement.firstChildElement("memory-segment");
            !element.isNull();
            element = element.nextSiblingElement("memory-segment")
        ) {
            auto section = TargetDescriptionFile::memorySegmentFromXml(element);
            output.memorySegmentsByKey.insert(std::pair(section.key, std::move(section)));
        }

        return output;
    }

    MemorySegment TargetDescriptionFile::memorySegmentFromXml(const QDomElement& xmlElement) {
        static const auto typesByName = BiMap<std::string, MemorySegmentType>({
            {"aliased", MemorySegmentType::ALIASED},
            {"regs", MemorySegmentType::REGISTERS},
            {"eeprom", MemorySegmentType::EEPROM},
            {"flash", MemorySegmentType::FLASH},
            {"fuses", MemorySegmentType::FUSES},
            {"io", MemorySegmentType::IO},
            {"ram", MemorySegmentType::RAM},
            {"lockbits", MemorySegmentType::LOCKBITS},
            {"osccal", MemorySegmentType::OSCCAL},
            {"production_signatures", MemorySegmentType::PRODUCTION_SIGNATURES},
            {"signatures", MemorySegmentType::SIGNATURES},
            {"user_signatures", MemorySegmentType::USER_SIGNATURES},
        });

        const auto typeName = TargetDescriptionFile::getAttribute(xmlElement, "type");

        const auto type = typesByName.valueAt(typeName);
        if (!type.has_value()) {
            throw InvalidTargetDescriptionDataException(
                "Failed to extract memory segment from TDF - invalid memory segment type name \"" + typeName + "\""
            );
        }

        const auto pageSize = TargetDescriptionFile::tryGetAttribute(xmlElement, "page-size");

        auto output = MemorySegment(
            TargetDescriptionFile::getAttribute(xmlElement, "key"),
            TargetDescriptionFile::getAttribute(xmlElement, "name"),
            *type,
            StringService::toUint32(TargetDescriptionFile::getAttribute(xmlElement, "start")),
            StringService::toUint32(TargetDescriptionFile::getAttribute(xmlElement, "size")),
            pageSize.has_value()
                ? std::optional(StringService::toUint16(*pageSize))
                : std::nullopt,
            {}
        );

        for (
            auto element = xmlElement.firstChildElement("section");
            !element.isNull();
            element = element.nextSiblingElement("section")
        ) {
            auto section = TargetDescriptionFile::memorySegmentSectionFromXml(element);
            output.sectionsByKey.insert(std::pair(section.key, std::move(section)));
        }

        return output;
    }

    MemorySegmentSection TargetDescriptionFile::memorySegmentSectionFromXml(const QDomElement& xmlElement) {
        auto output = MemorySegmentSection(
            TargetDescriptionFile::getAttribute(xmlElement, "key"),
            TargetDescriptionFile::getAttribute(xmlElement, "name"),
            StringService::toUint32(TargetDescriptionFile::getAttribute(xmlElement, "start")),
            StringService::toUint32(TargetDescriptionFile::getAttribute(xmlElement, "size")),
            {}
        );

        for (
            auto element = xmlElement.firstChildElement("section");
            !element.isNull();
            element = element.nextSiblingElement("section")
        ) {
            auto section = TargetDescriptionFile::memorySegmentSectionFromXml(element);
            output.subSectionsByKey.insert(std::pair(section.key, std::move(section)));
        }

        return output;
    }

    PhysicalInterface TargetDescriptionFile::physicalInterfaceFromXml(const QDomElement& xmlElement) {
        return PhysicalInterface(
            TargetDescriptionFile::getAttribute(xmlElement, "name"),
            TargetDescriptionFile::getAttribute(xmlElement, "type")
        );
    }

    Module TargetDescriptionFile::moduleFromXml(const QDomElement& xmlElement) {
        auto output = Module(
            TargetDescriptionFile::getAttribute(xmlElement, "key"),
            TargetDescriptionFile::getAttribute(xmlElement, "name"),
            TargetDescriptionFile::getAttribute(xmlElement, "description"),
            {}
        );

        for (
            auto element = xmlElement.firstChildElement("register-group");
            !element.isNull();
            element = element.nextSiblingElement("register-group")
        ) {
            auto registerGroup = TargetDescriptionFile::registerGroupFromXml(element);
            output.registerGroupsByKey.insert(std::pair(registerGroup.key, std::move(registerGroup)));
        }

        return output;
    }

    RegisterGroup TargetDescriptionFile::registerGroupFromXml(const QDomElement& xmlElement) {
        const auto offset = TargetDescriptionFile::tryGetAttribute(xmlElement, "offset");

        auto output = RegisterGroup(
            TargetDescriptionFile::getAttribute(xmlElement, "key"),
            TargetDescriptionFile::getAttribute(xmlElement, "name"),
            offset.has_value() ? std::optional(StringService::toUint32(*offset)) : std::nullopt,
            {},
            {},
            {}
        );

        for (
            auto element = xmlElement.firstChildElement("register");
            !element.isNull();
            element = element.nextSiblingElement("register")
        ) {
            auto reg = TargetDescriptionFile::registerFromXml(element);
            output.registersByKey.insert(std::pair(reg.key, std::move(reg)));
        }

        for (
            auto element = xmlElement.firstChildElement("register-group");
            !element.isNull();
            element = element.nextSiblingElement("register-group")
        ) {
            auto registerGroup = TargetDescriptionFile::registerGroupFromXml(element);
            output.subgroupsByKey.insert(std::pair(registerGroup.key, std::move(registerGroup)));
        }

        for (
            auto element = xmlElement.firstChildElement("register-group-reference");
            !element.isNull();
            element = element.nextSiblingElement("register-group-reference")
        ) {
            auto registerGroupReference = TargetDescriptionFile::registerGroupReferenceFromXml(element);
            output.subgroupReferencesByKey.insert(
                std::pair(registerGroupReference.key, std::move(registerGroupReference))
            );
        }

        return output;
    }

    RegisterGroupReference TargetDescriptionFile::registerGroupReferenceFromXml(const QDomElement& xmlElement) {
        return RegisterGroupReference(
            TargetDescriptionFile::getAttribute(xmlElement, "key"),
            TargetDescriptionFile::getAttribute(xmlElement, "name"),
            TargetDescriptionFile::getAttribute(xmlElement, "register-group-key"),
            StringService::toUint32(TargetDescriptionFile::getAttribute(xmlElement, "offset")),
            TargetDescriptionFile::tryGetAttribute(xmlElement, "description")
        );
    }

    Register TargetDescriptionFile::registerFromXml(const QDomElement& xmlElement) {
        const auto initialValue = TargetDescriptionFile::tryGetAttribute(xmlElement, "initial-value");
        const auto alternative = TargetDescriptionFile::tryGetAttribute(xmlElement, "alternative");

        auto output = Register(
            TargetDescriptionFile::getAttribute(xmlElement, "key"),
            TargetDescriptionFile::getAttribute(xmlElement, "name"),
            TargetDescriptionFile::tryGetAttribute(xmlElement, "description"),
            StringService::toUint32(TargetDescriptionFile::getAttribute(xmlElement, "offset")),
            StringService::toUint16(TargetDescriptionFile::getAttribute(xmlElement, "size")),
            initialValue.has_value() ? std::optional(StringService::toUint64(*initialValue)) : std::nullopt,
            TargetDescriptionFile::tryGetAttribute(xmlElement, "access"),
            alternative.has_value() ? std::optional(*alternative == "true") : std::nullopt,
            {}
        );

        for (
            auto element = xmlElement.firstChildElement("bit-field");
            !element.isNull();
            element = element.nextSiblingElement("bit-field")
        ) {
            auto bitField = TargetDescriptionFile::bitFieldFromXml(element);
            output.bitFieldsByKey.insert(std::pair(bitField.key, std::move(bitField)));
        }

        return output;
    }

    BitField TargetDescriptionFile::bitFieldFromXml(const QDomElement& xmlElement) {
        return BitField(
            TargetDescriptionFile::getAttribute(xmlElement, "key"),
            TargetDescriptionFile::getAttribute(xmlElement, "name"),
            TargetDescriptionFile::tryGetAttribute(xmlElement, "description"),
            StringService::toUint64(TargetDescriptionFile::getAttribute(xmlElement, "mask")),
            TargetDescriptionFile::tryGetAttribute(xmlElement, "access")
        );
    }

    Peripheral TargetDescriptionFile::peripheralFromXml(const QDomElement& xmlElement) {
        const auto offset = TargetDescriptionFile::tryGetAttribute(xmlElement, "offset");

        auto output = Peripheral(
            TargetDescriptionFile::getAttribute(xmlElement, "key"),
            TargetDescriptionFile::getAttribute(xmlElement, "name"),
            TargetDescriptionFile::getAttribute(xmlElement, "module-key"),
            {},
            {}
        );

        for (
            auto element = xmlElement.firstChildElement("register-group-instance");
            !element.isNull();
            element = element.nextSiblingElement("register-group-instance")
        ) {
            auto registerGroupInstance = TargetDescriptionFile::registerGroupInstanceFromXml(element);
            output.registerGroupInstancesByKey.insert(
                std::pair(registerGroupInstance.key, std::move(registerGroupInstance))
            );
        }

        for (
            auto element = xmlElement.firstChildElement("signals").firstChildElement("signal");
            !element.isNull();
            element = element.nextSiblingElement("signal")
        ) {
            output.sigs.emplace_back(TargetDescriptionFile::signalFromXml(element));
        }

        return output;
    }

    RegisterGroupInstance TargetDescriptionFile::registerGroupInstanceFromXml(const QDomElement& xmlElement) {
        return RegisterGroupInstance(
            TargetDescriptionFile::getAttribute(xmlElement, "key"),
            TargetDescriptionFile::getAttribute(xmlElement, "name"),
            TargetDescriptionFile::getAttribute(xmlElement, "register-group-key"),
            TargetDescriptionFile::getAttribute(xmlElement, "address-space-key"),
            StringService::toUint32(TargetDescriptionFile::getAttribute(xmlElement, "offset")),
            TargetDescriptionFile::tryGetAttribute(xmlElement, "description")
        );
    }

    Signal TargetDescriptionFile::signalFromXml(const QDomElement& xmlElement) {
        const auto index = TargetDescriptionFile::tryGetAttribute(xmlElement, "index");

        return Signal(
            TargetDescriptionFile::getAttribute(xmlElement, "pad-id"),
            index.has_value() ? std::optional(StringService::toUint64(*index)) : std::nullopt,
            TargetDescriptionFile::tryGetAttribute(xmlElement, "function"),
            TargetDescriptionFile::tryGetAttribute(xmlElement, "group"),
            TargetDescriptionFile::tryGetAttribute(xmlElement, "field")
        );
    }

    Pinout TargetDescriptionFile::pinoutFromXml(const QDomElement& xmlElement) {
        static const auto typesByName = BiMap<std::string, PinoutType>({
            {"soic", PinoutType::SOIC},
            {"ssop", PinoutType::SSOP},
            {"dip", PinoutType::DIP},
            {"qfn", PinoutType::QFN},
            {"mlf", PinoutType::MLF},
            {"drqfn", PinoutType::DUAL_ROW_QFN},
            {"qfp", PinoutType::QFP},
            {"bga", PinoutType::BGA},
        });

        const auto typeName = TargetDescriptionFile::getAttribute(xmlElement, "type");

        const auto type = typesByName.valueAt(typeName);
        if (!type.has_value()) {
            throw InvalidTargetDescriptionDataException(
                "Failed to extract pinout from TDF - invalid pinout type name \"" + typeName + "\""
            );
        }

        auto output = Pinout(
            TargetDescriptionFile::getAttribute(xmlElement, "key"),
            TargetDescriptionFile::getAttribute(xmlElement, "name"),
            *type,
            TargetDescriptionFile::tryGetAttribute(xmlElement, "function"),
            {}
        );

        for (
            auto element = xmlElement.firstChildElement("pin");
            !element.isNull();
            element = element.nextSiblingElement("pin")
        ) {
            output.pins.emplace_back(TargetDescriptionFile::pinFromXml(element));
        }

        return output;
    }

    Pin TargetDescriptionFile::pinFromXml(const QDomElement& xmlElement) {
        return Pin(
            TargetDescriptionFile::getAttribute(xmlElement, "position"),
            TargetDescriptionFile::getAttribute(xmlElement, "pad")
        );
    }

    Variant TargetDescriptionFile::variantFromXml(const QDomElement& xmlElement) {
        return Variant(
            TargetDescriptionFile::getAttribute(xmlElement, "name"),
            TargetDescriptionFile::getAttribute(xmlElement, "pinout-key"),
            TargetDescriptionFile::getAttribute(xmlElement, "package")
        );
    }
}
