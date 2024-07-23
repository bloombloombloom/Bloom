#include "TargetDescriptionFile.hpp"

#include <QFile>

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

    const std::string& TargetDescriptionFile::getName() const {
        return this->getDeviceAttribute("name");
    }

    TargetFamily TargetDescriptionFile::getFamily() const {
        const auto& familyName = this->getDeviceAttribute("family");

        if (familyName == "AVR8") {
            return TargetFamily::AVR_8;
        }

        if (familyName == "RISCV") {
            return TargetFamily::RISC_V;
        }

        throw InvalidTargetDescriptionDataException{"Failed to resolve target family - invalid family name"};
    }

    std::optional<std::string> TargetDescriptionFile::tryGetVendorName() const {
        return this->tryGetDeviceAttribute("vendor");
    }

    const std::string& TargetDescriptionFile::getVendorName() const {
        return this->getDeviceAttribute("vendor");
    }

    std::optional<std::reference_wrapper<const PropertyGroup>> TargetDescriptionFile::tryGetPropertyGroup(
        std::string_view keyStr
    ) const {
        const auto keys = StringService::split(keyStr, '.');

        const auto firstSubgroupIt = this->propertyGroupsByKey.find(*keys.begin());
        return firstSubgroupIt != this->propertyGroupsByKey.end()
            ? keys.size() > 1
                ? firstSubgroupIt->second.tryGetSubgroup(keys | std::ranges::views::drop(1))
                : std::optional{std::cref(firstSubgroupIt->second)}
            : std::nullopt;
    }

    const PropertyGroup& TargetDescriptionFile::getPropertyGroup(std::string_view keyStr) const {
        const auto propertyGroup = this->tryGetPropertyGroup(keyStr);

        if (!propertyGroup.has_value()) {
            throw InvalidTargetDescriptionDataException{
                "Failed to get property group \"" + std::string{keyStr} + "\" from TDF - property group not found"
            };
        }

        return propertyGroup->get();
    }

    std::optional<std::reference_wrapper<const Property>> TargetDescriptionFile::tryGetProperty(
        std::string_view groupKey,
        std::string_view propertyKey
    ) const {
        const auto propertyGroup = this->tryGetPropertyGroup(groupKey);

        if (!propertyGroup.has_value()) {
            return std::nullopt;
        }

        return propertyGroup->get().tryGetProperty(propertyKey);
    }

    const Property& TargetDescriptionFile::getProperty(std::string_view groupKey, std::string_view propertyKey) const {
        const auto property = this->tryGetProperty(groupKey, propertyKey);

        if (!property.has_value()) {
            throw InvalidTargetDescriptionDataException{
                "Failed to get property \"" + std::string{propertyKey} + "\" from group \"" + std::string{groupKey}
                    + "\", from TDF - property/group not found"
            };
        }

        return property->get();
    }

    std::optional<std::reference_wrapper<const AddressSpace>> TargetDescriptionFile::tryGetAddressSpace(
        std::string_view key
    ) const {
        const auto addressSpaceIt = this->addressSpacesByKey.find(key);
        return addressSpaceIt != this->addressSpacesByKey.end()
            ? std::optional{std::cref(addressSpaceIt->second)}
            : std::nullopt;
    }

    const AddressSpace& TargetDescriptionFile::getAddressSpace(std::string_view key) const {
        const auto addressSpace = this->tryGetAddressSpace(key);

        if (!addressSpace.has_value()) {
            throw InvalidTargetDescriptionDataException{
                "Failed to get address space \"" + std::string{key} + "\" from TDF - address space not found"
            };
        }

        return addressSpace->get();
    }

    std::optional<std::reference_wrapper<const MemorySegment>> TargetDescriptionFile::tryGetMemorySegment(
        std::string_view addressSpaceKey,
        std::string_view segmentKey
    ) const {
        const auto addressSpace = this->tryGetAddressSpace(addressSpaceKey);

        if (!addressSpace.has_value()) {
            return std::nullopt;
        }

        return addressSpace->get().tryGetMemorySegment(segmentKey);
    }

    const MemorySegment& TargetDescriptionFile::getMemorySegment(
        std::string_view addressSpaceKey,
        std::string_view segmentKey
    ) const {
        const auto segment = this->tryGetMemorySegment(addressSpaceKey, segmentKey);

        if (!segment.has_value()) {
            throw InvalidTargetDescriptionDataException{
                "Failed to get memory segment \"" + std::string{segmentKey} + "\" from address space \""
                    + std::string{addressSpaceKey} + "\" from TDF"
            };
        }

        return segment->get();
    }

    std::set<TargetPhysicalInterface> TargetDescriptionFile::getPhysicalInterfaces() const {
        static const auto physicalInterfacesByValue = BiMap<std::string, TargetPhysicalInterface>{
            {"updi", TargetPhysicalInterface::UPDI},
            {"debug_wire", TargetPhysicalInterface::DEBUG_WIRE},
            {"jtag", TargetPhysicalInterface::JTAG},
            {"pdi", TargetPhysicalInterface::PDI},
            {"isp", TargetPhysicalInterface::ISP},
        };

        auto output = std::set<TargetPhysicalInterface>{};

        for (const auto& physicalInterface : this->physicalInterfaces) {
            const auto interface = physicalInterfacesByValue.valueAt(physicalInterface.value);

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
            ? std::optional{std::cref(moduleIt->second)}
            : std::nullopt;
    }

    const Module& TargetDescriptionFile::getModule(std::string_view key) const {
        const auto module = this->tryGetModule(key);

        if (!module.has_value()) {
            throw InvalidTargetDescriptionDataException{
                "Failed to get module \"" + std::string{key} + "\" from TDF - module not found"
            };
        }

        return module->get();
    }

    std::optional<std::reference_wrapper<const Peripheral>> TargetDescriptionFile::tryGetPeripheral(
        std::string_view key
    ) const {
        const auto peripheralIt = this->peripheralsByKey.find(key);
        return peripheralIt != this->peripheralsByKey.end()
            ? std::optional{std::cref(peripheralIt->second)}
            : std::nullopt;
    }

    const Peripheral& TargetDescriptionFile::getPeripheral(std::string_view key) const {
        const auto peripheral = this->tryGetPeripheral(key);

        if (!peripheral.has_value()) {
            throw InvalidTargetDescriptionDataException{
                "Failed to get peripheral \"" + std::string{key} + "\" from TDF - peripheral not found"
            };
        }

        return peripheral->get();
    }

    std::optional<TargetMemorySegmentDescriptor> TargetDescriptionFile::tryGetTargetMemorySegmentDescriptor(
        std::string_view addressSpaceKey,
        std::string_view segmentKey
    ) const {
        const auto addressSpace = this->tryGetAddressSpace(addressSpaceKey);

        if (!addressSpace.has_value()) {
            return std::nullopt;
        }

        const auto segment = addressSpace->get().tryGetMemorySegment(segmentKey);

        if (!segment.has_value()) {
            return std::nullopt;
        }

        return TargetDescriptionFile::targetMemorySegmentDescriptorFromMemorySegment(
            segment->get(),
            addressSpace->get()
        );
    }

    TargetMemorySegmentDescriptor TargetDescriptionFile::getTargetMemorySegmentDescriptor(
        std::string_view addressSpaceKey,
        std::string_view segmentKey
    ) const {
        const auto& addressSpace = this->getAddressSpace(addressSpaceKey);
        return TargetDescriptionFile::targetMemorySegmentDescriptorFromMemorySegment(
            addressSpace.getMemorySegment(segmentKey),
            addressSpace
        );
    }

    std::optional<TargetPeripheralDescriptor> TargetDescriptionFile::tryGetTargetPeripheralDescriptor(
        std::string_view key
    ) const {
        const auto peripheral = this->tryGetPeripheral(key);

        if (!peripheral.has_value()) {
            return std::nullopt;
        }

        return TargetDescriptionFile::targetPeripheralDescriptorFromPeripheral(
            peripheral->get(),
            this->getModule(peripheral->get().moduleKey)
        );
    }

    TargetPeripheralDescriptor TargetDescriptionFile::getTargetPeripheralDescriptor(std::string_view key) const {
        const auto& peripheral = this->getPeripheral(key);

        return TargetDescriptionFile::targetPeripheralDescriptorFromPeripheral(
            peripheral,
            this->getModule(peripheral.moduleKey)
        );
    }

    std::map<
        std::string,
        TargetAddressSpaceDescriptor
    > TargetDescriptionFile::targetAddressSpaceDescriptorsByKey() const {
        auto output = std::map<std::string, TargetAddressSpaceDescriptor>{};

        for (const auto& [key, addressSpace] : this->addressSpacesByKey) {
            output.emplace(key, TargetDescriptionFile::targetAddressSpaceDescriptorFromAddressSpace(addressSpace));
        }

        return output;
    }

    std::map<std::string, TargetPeripheralDescriptor> TargetDescriptionFile::targetPeripheralDescriptorsByKey() const {
        auto output = std::map<std::string, TargetPeripheralDescriptor>{};

        for (const auto& [key, peripheral] : this->peripheralsByKey) {
            output.emplace(
                key,
                TargetDescriptionFile::targetPeripheralDescriptorFromPeripheral(
                    peripheral,
                    this->getModule(peripheral.moduleKey)
                )
            );
        }

        return output;
    }

    std::map<std::string, TargetPinoutDescriptor> TargetDescriptionFile::targetPinoutDescriptorsByKey() const {
        auto output = std::map<std::string, TargetPinoutDescriptor>{};

        for (const auto& [key, pinout] : this->pinoutsByKey) {
            output.emplace(
                key,
                TargetDescriptionFile::targetPinoutDescriptorFromPinout(pinout)
            );
        }

        return output;
    }

    std::vector<TargetVariantDescriptor> TargetDescriptionFile::targetVariantDescriptors() const {
        auto output = std::vector<TargetVariantDescriptor>{};

        for (const auto& variant : this->variants) {
            output.emplace_back(TargetDescriptionFile::targetVariantDescriptorFromVariant(variant));
        }

        return output;
    }

    std::vector<TargetPeripheralDescriptor> TargetDescriptionFile::gpioPortPeripheralDescriptors() const {
        auto output = std::vector<TargetPeripheralDescriptor>{};

        const auto& gpioPortModule = this->getModule("gpio_port");

        for (const auto& [peripheralKey, peripheral] : this->peripheralsByKey) {
            if (peripheral.moduleKey != gpioPortModule.key) {
                continue;
            }

            output.emplace_back(
                TargetDescriptionFile::targetPeripheralDescriptorFromPeripheral(peripheral, gpioPortModule)
            );
        }

        return output;
    }

    void TargetDescriptionFile::init(const std::string& xmlFilePath) {
        auto file = QFile{QString::fromStdString(xmlFilePath)};
        if (!file.exists()) {
            throw InternalFatalErrorException{"Failed to load target description file - file not found"};
        }

        file.open(QIODevice::ReadOnly);
        auto document = QDomDocument{};
        if (!document.setContent(file.readAll())) {
            throw TargetDescriptionParsingFailureException{};
        }

        this->init(document);
    }

    void TargetDescriptionFile::init(const QDomDocument& document) {
        const auto deviceElement = document.documentElement();
        if (deviceElement.nodeName() != "device") {
            throw TargetDescriptionParsingFailureException{"Root \"device\" element not found."};
        }

        const auto deviceAttributes = deviceElement.attributes();
        for (auto i = 0; i < deviceAttributes.length(); ++i) {
            const auto deviceAttribute = deviceAttributes.item(i);
            this->deviceAttributesByName.emplace(
                deviceAttribute.nodeName().toStdString(),
                deviceAttribute.nodeValue().toStdString()
            );
        }

        for (
            auto element = deviceElement.firstChildElement("property-groups").firstChildElement("property-group");
            !element.isNull();
            element = element.nextSiblingElement("property-group")
        ) {
            auto propertyGroup = TargetDescriptionFile::propertyGroupFromXml(element);
            this->propertyGroupsByKey.emplace(propertyGroup.key, std::move(propertyGroup));
        }

        for (
            auto element = deviceElement.firstChildElement("address-spaces").firstChildElement("address-space");
            !element.isNull();
            element = element.nextSiblingElement("address-space")
        ) {
            auto addressSpace = TargetDescriptionFile::addressSpaceFromXml(element);
            this->addressSpacesByKey.emplace(addressSpace.key, std::move(addressSpace));
        }

        for (
            auto element = deviceElement.firstChildElement("physical-interfaces")
                .firstChildElement("physical-interface");
            !element.isNull();
            element = element.nextSiblingElement("physical-interface")
        ) {
            this->physicalInterfaces.emplace_back(TargetDescriptionFile::physicalInterfaceFromXml(element));
        }

        for (
            auto element = deviceElement.firstChildElement("modules").firstChildElement("module");
            !element.isNull();
            element = element.nextSiblingElement("module")
        ) {
            auto module = TargetDescriptionFile::moduleFromXml(element);
            this->modulesByKey.emplace(module.key, std::move(module));
        }

        for (
            auto element = deviceElement.firstChildElement("peripherals").firstChildElement("peripheral");
            !element.isNull();
            element = element.nextSiblingElement("peripheral")
        ) {
            auto peripheral = TargetDescriptionFile::peripheralFromXml(element);
            this->peripheralsByKey.emplace(peripheral.key, std::move(peripheral));
        }

        for (
            auto element = deviceElement.firstChildElement("pinouts").firstChildElement("pinout");
            !element.isNull();
            element = element.nextSiblingElement("pinout")
        ) {
            auto pinout = TargetDescriptionFile::pinoutFromXml(element);
            this->pinoutsByKey.emplace(pinout.key, std::move(pinout));
        }

        for (
            auto element = deviceElement.firstChildElement("variants").firstChildElement("variant");
            !element.isNull();
            element = element.nextSiblingElement("variant")
        ) {
            this->variants.emplace_back(TargetDescriptionFile::variantFromXml(element));
        }
    }

    std::optional<std::reference_wrapper<const std::string>> TargetDescriptionFile::tryGetDeviceAttribute(
        const std::string& attributeName
    ) const {
        const auto attributeIt = this->deviceAttributesByName.find(attributeName);

        if (attributeIt == this->deviceAttributesByName.end()) {
            return std::nullopt;
        }

        return std::cref(attributeIt->second);
    }

    const std::string& TargetDescriptionFile::getDeviceAttribute(const std::string& attributeName) const {
        const auto attribute = this->tryGetDeviceAttribute(attributeName);

        if (!attribute.has_value()) {
            throw InvalidTargetDescriptionDataException{"Missing target device attribute (\"" + attributeName + "\")"};
        }

        return attribute->get();
    }

    std::optional<std::string> TargetDescriptionFile::tryGetAttribute(
        const QDomElement& element,
        const QString& attributeName
    ) {
        return element.hasAttribute(attributeName)
            ? std::optional{element.attribute(attributeName).toStdString()}
            : std::nullopt;
    }

    std::string TargetDescriptionFile::getAttribute(const QDomElement& element, const QString& attributeName) {
        const auto attribute = TargetDescriptionFile::tryGetAttribute(element, attributeName);

        if (!attribute.has_value()) {
            throw InvalidTargetDescriptionDataException{
                "Failed to fetch attribute from TDF element \"" + element.nodeName().toStdString()
                    + "\" - attribute \"" + attributeName.toStdString() + "\" not found"
            };
        }

        return *attribute;
    }

    PropertyGroup TargetDescriptionFile::propertyGroupFromXml(const QDomElement& xmlElement) {
        auto output = PropertyGroup{TargetDescriptionFile::getAttribute(xmlElement, "key"), {}, {}};

        for (
            auto element = xmlElement.firstChildElement("property");
            !element.isNull();
            element = element.nextSiblingElement("property")
        ) {
            auto property = TargetDescriptionFile::propertyFromXml(element);
            output.propertiesByKey.emplace(property.key, std::move(property));
        }

        for (
            auto element = xmlElement.firstChildElement("property-group");
            !element.isNull();
            element = element.nextSiblingElement("property-group")
        ) {
            auto subgroup = TargetDescriptionFile::propertyGroupFromXml(element);
            output.subgroupsByKey.emplace(subgroup.key, std::move(subgroup));
        }

        return output;
    }

    Property TargetDescriptionFile::propertyFromXml(const QDomElement& xmlElement) {
        return {
            TargetDescriptionFile::getAttribute(xmlElement, "key"),
            TargetDescriptionFile::getAttribute(xmlElement, "value")
        };
    }

    AddressSpace TargetDescriptionFile::addressSpaceFromXml(const QDomElement& xmlElement) {
        static const auto endiannessByName = BiMap<std::string, TargetMemoryEndianness>{
            {"big", TargetMemoryEndianness::BIG},
            {"little", TargetMemoryEndianness::LITTLE},
        };

        const auto endiannessName = TargetDescriptionFile::tryGetAttribute(xmlElement, "endianness");

        auto endianness = std::optional<TargetMemoryEndianness>{};
        if (endiannessName.has_value()) {
            endianness = endiannessByName.valueAt(*endiannessName);

            if (!endianness.has_value()) {
                throw InvalidTargetDescriptionDataException{
                    "Failed to extract address space from TDF - invalid endianness name \"" + *endiannessName + "\""
                };
            }
        }

        const auto unitSize = TargetDescriptionFile::tryGetAttribute(xmlElement, "unit-size");

        auto output = AddressSpace{
            TargetDescriptionFile::getAttribute(xmlElement, "key"),
            StringService::toUint32(TargetDescriptionFile::getAttribute(xmlElement, "start")),
            StringService::toUint32(TargetDescriptionFile::getAttribute(xmlElement, "size")),
            unitSize.has_value() ? StringService::toUint8(*unitSize) : std::uint8_t{1},
            endianness,
            {}
        };

        for (
            auto element = xmlElement.firstChildElement("memory-segment");
            !element.isNull();
            element = element.nextSiblingElement("memory-segment")
        ) {
            auto section = TargetDescriptionFile::memorySegmentFromXml(element);
            output.memorySegmentsByKey.emplace(section.key, std::move(section));
        }

        return output;
    }

    MemorySegment TargetDescriptionFile::memorySegmentFromXml(const QDomElement& xmlElement) {
        static const auto typesByName = BiMap<std::string, TargetMemorySegmentType>{
            {"gp_registers", TargetMemorySegmentType::GENERAL_PURPOSE_REGISTERS},
            {"aliased", TargetMemorySegmentType::ALIASED},
            {"regs", TargetMemorySegmentType::REGISTERS},
            {"eeprom", TargetMemorySegmentType::EEPROM},
            {"flash", TargetMemorySegmentType::FLASH},
            {"fuses", TargetMemorySegmentType::FUSES},
            {"io", TargetMemorySegmentType::IO},
            {"ram", TargetMemorySegmentType::RAM},
            {"lockbits", TargetMemorySegmentType::LOCKBITS},
            {"osccal", TargetMemorySegmentType::OSCCAL},
            {"production_signatures", TargetMemorySegmentType::PRODUCTION_SIGNATURES},
            {"signatures", TargetMemorySegmentType::SIGNATURES},
            {"user_signatures", TargetMemorySegmentType::USER_SIGNATURES},
        };

        const auto typeName = TargetDescriptionFile::getAttribute(xmlElement, "type");

        const auto type = typesByName.valueAt(typeName);
        if (!type.has_value()) {
            throw InvalidTargetDescriptionDataException{
                "Failed to extract memory segment from TDF - invalid memory segment type name \"" + typeName + "\""
            };
        }

        const auto pageSize = TargetDescriptionFile::tryGetAttribute(xmlElement, "page-size");
        const auto accessString = TargetDescriptionFile::tryGetAttribute(xmlElement, "access");

        auto output = MemorySegment{
            TargetDescriptionFile::getAttribute(xmlElement, "key"),
            TargetDescriptionFile::getAttribute(xmlElement, "name"),
            *type,
            StringService::toUint32(TargetDescriptionFile::getAttribute(xmlElement, "start")),
            StringService::toUint32(TargetDescriptionFile::getAttribute(xmlElement, "size")),
            TargetDescriptionFile::getAttribute(xmlElement, "executable") == "1",
            TargetMemoryAccess{
                accessString.has_value() ? accessString->find('R') != std::string::npos : true,
                accessString.has_value() ? accessString->find('W') != std::string::npos : true
            },
            pageSize.has_value()
                ? std::optional{StringService::toUint32(*pageSize)}
                : std::nullopt,
            {}
        };

        for (
            auto element = xmlElement.firstChildElement("section");
            !element.isNull();
            element = element.nextSiblingElement("section")
        ) {
            auto section = TargetDescriptionFile::memorySegmentSectionFromXml(element);
            output.sectionsByKey.emplace(section.key, std::move(section));
        }

        return output;
    }

    MemorySegmentSection TargetDescriptionFile::memorySegmentSectionFromXml(const QDomElement& xmlElement) {
        auto output = MemorySegmentSection{
            TargetDescriptionFile::getAttribute(xmlElement, "key"),
            TargetDescriptionFile::getAttribute(xmlElement, "name"),
            StringService::toUint32(TargetDescriptionFile::getAttribute(xmlElement, "start")),
            StringService::toUint32(TargetDescriptionFile::getAttribute(xmlElement, "size")),
            {}
        };

        for (
            auto element = xmlElement.firstChildElement("section");
            !element.isNull();
            element = element.nextSiblingElement("section")
        ) {
            auto section = TargetDescriptionFile::memorySegmentSectionFromXml(element);
            output.subSectionsByKey.emplace(section.key, std::move(section));
        }

        return output;
    }

    PhysicalInterface TargetDescriptionFile::physicalInterfaceFromXml(const QDomElement& xmlElement) {
        return {
            TargetDescriptionFile::getAttribute(xmlElement, "value")
        };
    }

    Module TargetDescriptionFile::moduleFromXml(const QDomElement& xmlElement) {
        auto output = Module{
            TargetDescriptionFile::getAttribute(xmlElement, "key"),
            TargetDescriptionFile::getAttribute(xmlElement, "name"),
            TargetDescriptionFile::getAttribute(xmlElement, "description"),
            {}
        };

        for (
            auto element = xmlElement.firstChildElement("register-group");
            !element.isNull();
            element = element.nextSiblingElement("register-group")
        ) {
            auto registerGroup = TargetDescriptionFile::registerGroupFromXml(element);
            output.registerGroupsByKey.emplace(registerGroup.key, std::move(registerGroup));
        }

        return output;
    }

    RegisterGroup TargetDescriptionFile::registerGroupFromXml(const QDomElement& xmlElement) {
        const auto offset = TargetDescriptionFile::tryGetAttribute(xmlElement, "offset");

        auto output = RegisterGroup{
            TargetDescriptionFile::getAttribute(xmlElement, "key"),
            TargetDescriptionFile::getAttribute(xmlElement, "name"),
            offset.has_value() ? std::optional{StringService::toUint32(*offset)} : std::nullopt,
            {},
            {},
            {}
        };

        for (
            auto element = xmlElement.firstChildElement("register");
            !element.isNull();
            element = element.nextSiblingElement("register")
        ) {
            auto reg = TargetDescriptionFile::registerFromXml(element);
            output.registersByKey.emplace(reg.key, std::move(reg));
        }

        for (
            auto element = xmlElement.firstChildElement("register-group");
            !element.isNull();
            element = element.nextSiblingElement("register-group")
        ) {
            auto registerGroup = TargetDescriptionFile::registerGroupFromXml(element);
            output.subgroupsByKey.emplace(registerGroup.key, std::move(registerGroup));
        }

        for (
            auto element = xmlElement.firstChildElement("register-group-reference");
            !element.isNull();
            element = element.nextSiblingElement("register-group-reference")
        ) {
            auto registerGroupReference = TargetDescriptionFile::registerGroupReferenceFromXml(element);
            output.subgroupReferencesByKey.emplace(registerGroupReference.key, std::move(registerGroupReference));
        }

        return output;
    }

    RegisterGroupReference TargetDescriptionFile::registerGroupReferenceFromXml(const QDomElement& xmlElement) {
        return {
            TargetDescriptionFile::getAttribute(xmlElement, "key"),
            TargetDescriptionFile::getAttribute(xmlElement, "name"),
            TargetDescriptionFile::getAttribute(xmlElement, "register-group-key"),
            StringService::toUint32(TargetDescriptionFile::getAttribute(xmlElement, "offset")),
            TargetDescriptionFile::tryGetAttribute(xmlElement, "description")
        };
    }

    Register TargetDescriptionFile::registerFromXml(const QDomElement& xmlElement) {
        const auto initialValue = TargetDescriptionFile::tryGetAttribute(xmlElement, "initial-value");
        const auto alternative = TargetDescriptionFile::tryGetAttribute(xmlElement, "alternative");
        const auto accessString = TargetDescriptionFile::tryGetAttribute(xmlElement, "access");

        auto output = Register{
            TargetDescriptionFile::getAttribute(xmlElement, "key"),
            TargetDescriptionFile::getAttribute(xmlElement, "name"),
            TargetDescriptionFile::tryGetAttribute(xmlElement, "description"),
            StringService::toUint32(TargetDescriptionFile::getAttribute(xmlElement, "offset")),
            StringService::toUint16(TargetDescriptionFile::getAttribute(xmlElement, "size")),
            initialValue.has_value() ? std::optional{StringService::toUint64(*initialValue)} : std::nullopt,
            accessString.has_value()
                ? std::optional{TargetRegisterAccess{
                    accessString->find('R') != std::string::npos,
                    accessString->find('W') != std::string::npos
                }}
                : std::nullopt,
            alternative.has_value() ? std::optional{*alternative == "true"} : std::nullopt,
            {}
        };

        for (
            auto element = xmlElement.firstChildElement("bit-field");
            !element.isNull();
            element = element.nextSiblingElement("bit-field")
        ) {
            auto bitField = TargetDescriptionFile::bitFieldFromXml(element);
            output.bitFieldsByKey.emplace(bitField.key, std::move(bitField));
        }

        return output;
    }

    BitField TargetDescriptionFile::bitFieldFromXml(const QDomElement& xmlElement) {
        return {
            TargetDescriptionFile::getAttribute(xmlElement, "key"),
            TargetDescriptionFile::getAttribute(xmlElement, "name"),
            TargetDescriptionFile::tryGetAttribute(xmlElement, "description"),
            StringService::toUint64(TargetDescriptionFile::getAttribute(xmlElement, "mask")),
            TargetDescriptionFile::tryGetAttribute(xmlElement, "access")
        };
    }

    Peripheral TargetDescriptionFile::peripheralFromXml(const QDomElement& xmlElement) {
        const auto offset = TargetDescriptionFile::tryGetAttribute(xmlElement, "offset");

        auto output = Peripheral{
            TargetDescriptionFile::getAttribute(xmlElement, "key"),
            TargetDescriptionFile::getAttribute(xmlElement, "name"),
            TargetDescriptionFile::getAttribute(xmlElement, "module-key"),
            {},
            {}
        };

        for (
            auto element = xmlElement.firstChildElement("register-group-instance");
            !element.isNull();
            element = element.nextSiblingElement("register-group-instance")
        ) {
            auto registerGroupInstance = TargetDescriptionFile::registerGroupInstanceFromXml(element);
            output.registerGroupInstancesByKey.emplace(registerGroupInstance.key, std::move(registerGroupInstance));
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
        return {
            TargetDescriptionFile::getAttribute(xmlElement, "key"),
            TargetDescriptionFile::getAttribute(xmlElement, "name"),
            TargetDescriptionFile::getAttribute(xmlElement, "register-group-key"),
            TargetDescriptionFile::getAttribute(xmlElement, "address-space-key"),
            StringService::toUint32(TargetDescriptionFile::getAttribute(xmlElement, "offset")),
            TargetDescriptionFile::tryGetAttribute(xmlElement, "description")
        };
    }

    Signal TargetDescriptionFile::signalFromXml(const QDomElement& xmlElement) {
        const auto index = TargetDescriptionFile::tryGetAttribute(xmlElement, "index");

        return {
            TargetDescriptionFile::getAttribute(xmlElement, "pad-id"),
            index.has_value() ? std::optional{StringService::toUint64(*index)} : std::nullopt,
            TargetDescriptionFile::tryGetAttribute(xmlElement, "function"),
            TargetDescriptionFile::tryGetAttribute(xmlElement, "group"),
            TargetDescriptionFile::tryGetAttribute(xmlElement, "field")
        };
    }

    Pinout TargetDescriptionFile::pinoutFromXml(const QDomElement& xmlElement) {
        static const auto typesByName = BiMap<std::string, TargetPinoutType>{
            {"soic", TargetPinoutType::SOIC},
            {"ssop", TargetPinoutType::SSOP},
            {"dip", TargetPinoutType::DIP},
            {"qfn", TargetPinoutType::QFN},
            {"mlf", TargetPinoutType::MLF},
            {"drqfn", TargetPinoutType::DUAL_ROW_QFN},
            {"qfp", TargetPinoutType::QFP},
            {"bga", TargetPinoutType::BGA},
        };

        const auto typeName = TargetDescriptionFile::getAttribute(xmlElement, "type");

        const auto type = typesByName.valueAt(typeName);
        if (!type.has_value()) {
            throw InvalidTargetDescriptionDataException{
                "Failed to extract pinout from TDF - invalid pinout type name \"" + typeName + "\""
            };
        }

        auto output = Pinout{
            TargetDescriptionFile::getAttribute(xmlElement, "key"),
            TargetDescriptionFile::getAttribute(xmlElement, "name"),
            *type,
            TargetDescriptionFile::tryGetAttribute(xmlElement, "function"),
            {}
        };

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
        return {
            TargetDescriptionFile::getAttribute(xmlElement, "position"),
            TargetDescriptionFile::getAttribute(xmlElement, "pad")
        };
    }

    Variant TargetDescriptionFile::variantFromXml(const QDomElement& xmlElement) {
        return {
            TargetDescriptionFile::getAttribute(xmlElement, "name"),
            TargetDescriptionFile::getAttribute(xmlElement, "pinout-key"),
            TargetDescriptionFile::getAttribute(xmlElement, "package")
        };
    }

    TargetAddressSpaceDescriptor TargetDescriptionFile::targetAddressSpaceDescriptorFromAddressSpace(
        const AddressSpace& addressSpace
    ) {
        auto output = TargetAddressSpaceDescriptor{
            addressSpace.key,
            TargetMemoryAddressRange{
                addressSpace.startAddress,
                addressSpace.startAddress + addressSpace.size - 1
            },
            addressSpace.endianness.value_or(TargetMemoryEndianness::LITTLE),
            {}
        };

        for (const auto& [key, memorySegment] : addressSpace.memorySegmentsByKey) {
            output.segmentDescriptorsByKey.emplace(
                key,
                TargetDescriptionFile::targetMemorySegmentDescriptorFromMemorySegment(memorySegment, addressSpace)
            );
        }

        return output;
    }

    TargetMemorySegmentDescriptor TargetDescriptionFile::targetMemorySegmentDescriptorFromMemorySegment(
        const MemorySegment& memorySegment,
        const AddressSpace& addressSpace
    ) {
        return {
            addressSpace.key,
            memorySegment.key,
            memorySegment.name,
            memorySegment.type,
            TargetMemoryAddressRange{
                memorySegment.startAddress,
                memorySegment.startAddress + (memorySegment.size / addressSpace.unitSize) - 1
            },
            addressSpace.unitSize,
            memorySegment.executable,
            memorySegment.access,
            memorySegment.access,
            memorySegment.pageSize
        };
    }

    TargetPeripheralDescriptor TargetDescriptionFile::targetPeripheralDescriptorFromPeripheral(
        const Peripheral& peripheral,
        const Module& peripheralModule
    ) {
        auto output = TargetPeripheralDescriptor{
            peripheral.key,
            peripheral.name,
            {},
            {}
        };

        for (const auto& [key, registerGroupInstance] : peripheral.registerGroupInstancesByKey) {
            output.registerGroupDescriptorsByKey.emplace(
                key,
                TargetDescriptionFile::targetRegisterGroupDescriptorFromRegisterGroup(
                    registerGroupInstance.key,
                    registerGroupInstance.name,
                    registerGroupInstance.addressSpaceKey,
                    registerGroupInstance.description,
                    peripheralModule.getRegisterGroup(registerGroupInstance.registerGroupKey),
                    peripheralModule,
                    registerGroupInstance.offset
                )
            );
        }

        for (const auto& signal : peripheral.sigs) {
            output.signalDescriptors.emplace_back(
                TargetDescriptionFile::targetPeripheralSignalDescriptorFromSignal(signal)
            );
        }

        return output;
    }

    TargetPeripheralSignalDescriptor TargetDescriptionFile::targetPeripheralSignalDescriptorFromSignal(
        const Signal& signal
    ) {
        return {
            signal.padId,
            signal.index
        };
    }

    TargetRegisterGroupDescriptor TargetDescriptionFile::targetRegisterGroupDescriptorFromRegisterGroup(
        const std::string& key,
        const std::string& name,
        const std::string& addressSpaceKey,
        const std::optional<std::string>& description,
        const RegisterGroup& registerGroup,
        const Module& peripheralModule,
        TargetMemoryAddress baseAddress
    ) {
        auto output = TargetRegisterGroupDescriptor{
            key,
            name,
            addressSpaceKey,
            description,
            {},
            {}
        };

        for (const auto& [key, subgroup] : registerGroup.subgroupsByKey) {
            output.subgroupDescriptorsByKey.emplace(
                key,
                TargetDescriptionFile::targetRegisterGroupDescriptorFromRegisterGroup(
                    subgroup.key,
                    subgroup.name,
                    addressSpaceKey,
                    std::nullopt,
                    registerGroup,
                    peripheralModule,
                    baseAddress + registerGroup.offset.value_or(0)
                )
            );
        }

        for (const auto& [key, subgroupReference] : registerGroup.subgroupReferencesByKey) {
            const auto& registerGroup = peripheralModule.getRegisterGroup(subgroupReference.registerGroupKey);
            output.subgroupDescriptorsByKey.emplace(
                key,
                TargetDescriptionFile::targetRegisterGroupDescriptorFromRegisterGroup(
                    subgroupReference.key,
                    subgroupReference.name,
                    addressSpaceKey,
                    subgroupReference.description,
                    registerGroup,
                    peripheralModule,
                    baseAddress + subgroupReference.offset + registerGroup.offset.value_or(0)
                )
            );
        }

        for (const auto& [key, reg] : registerGroup.registersByKey) {
            output.registerDescriptorsByKey.emplace(
                key,
                TargetDescriptionFile::targetRegisterDescriptorFromRegister(
                    reg,
                    addressSpaceKey,
                    baseAddress + registerGroup.offset.value_or(0)
                )
            );
        }

        return output;
    }

    TargetRegisterDescriptor TargetDescriptionFile::targetRegisterDescriptorFromRegister(
        const Register& reg,
        const std::string& addressSpaceKey,
        TargetMemoryAddress baseAddress
    ) {
        auto output = TargetRegisterDescriptor{
            reg.key,
            reg.name,
            addressSpaceKey,
            baseAddress + reg.offset,
            reg.size,
            TargetRegisterType::OTHER,
            reg.access.value_or(TargetRegisterAccess{true, true}),
            reg.description,
            {}
        };

        for (const auto& [key, bitField] : reg.bitFieldsByKey) {
            output.bitFieldDescriptorsByKey.emplace(
                key,
                TargetDescriptionFile::targetBitFieldDescriptorFromBitField(bitField)
            );
        }

        return output;
    }

    TargetBitFieldDescriptor TargetDescriptionFile::targetBitFieldDescriptorFromBitField(const BitField& bitField) {
        return {
            bitField.key,
            bitField.name,
            bitField.mask,
            bitField.description
        };
    }

    TargetPinoutDescriptor TargetDescriptionFile::targetPinoutDescriptorFromPinout(const Pinout& pinout) {
        auto output = TargetPinoutDescriptor{
            pinout.key,
            pinout.name,
            pinout.type,
            {}
        };

        for (const auto& pin : pinout.pins) {
            output.pinDescriptors.emplace_back(TargetDescriptionFile::targetPinDescriptorFromPin(pin));
        }

        return output;
    }

    TargetPinDescriptor TargetDescriptionFile::targetPinDescriptorFromPin(const Pin& pin) {
        static const auto resolvePinType = [] (const std::string& pad) -> TargetPinType {
            if (
                pad.find("vcc") == 0
                || pad.find("avcc") == 0
                || pad.find("aref") == 0
                || pad.find("avdd") == 0
                || pad.find("vdd") == 0
            ) {
                return TargetPinType::VCC;
            }

            if (pad.find("gnd") == 0) {
                return TargetPinType::GND;
            }

            return TargetPinType::OTHER;
        };

        return {
            pin.pad,
            pin.position,
            resolvePinType(pin.pad)
        };
    }

    TargetVariantDescriptor TargetDescriptionFile::targetVariantDescriptorFromVariant(const Variant& variant) {
        return {
            variant.name,
            variant.pinoutKey
        };
    }
}
