#include "TargetDescriptionFile.hpp"

#include <QJsonDocument>
#include <QJsonArray>

#include "src/Services/PathService.hpp"
#include "src/Services/StringService.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Helpers/BiMap.hpp"
#include "src/Logger/Logger.hpp"

#include "src/Exceptions/Exception.hpp"
#include "Exceptions/TargetDescriptionParsingFailureException.hpp"

namespace Targets::TargetDescription
{
    using namespace Exceptions;

    using Services::StringService;

    const std::map<std::string, GeneratedMapping::BriefTargetDescriptor>& TargetDescriptionFile::mapping() {
        return GeneratedMapping::map;
    }

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
        static const auto familiesByName = std::map<std::string, TargetFamily> {
            {"AVR8", TargetFamily::AVR_8},
            {"RISCV", TargetFamily::RISC_V},
        };

        const auto familyIt = familiesByName.find(this->deviceAttribute("family"));

        if (familyIt == familiesByName.end()) {
            throw Exception("Failed to resolve target family - invalid family name");
        }

        return familyIt->second;
    }

    std::optional<std::reference_wrapper<const PropertyGroup>> TargetDescriptionFile::tryGetPropertyGroup(
        std::string_view keyStr
    ) const {
        auto keys = StringService::split(keyStr, '.');

        const auto firstSubgroupIt = this->propertyGroupsMappedByKey.find(*keys.begin());
        return firstSubgroupIt != this->propertyGroupsMappedByKey.end()
            ? keys.size() > 1
                ? firstSubgroupIt->second.tryGetSubgroup(keys | std::ranges::views::drop(1))
                : std::optional(std::cref(firstSubgroupIt->second))
            : std::nullopt;
    }

    const PropertyGroup& TargetDescriptionFile::getPropertyGroup(std::string_view keyStr) const {
        const auto propertyGroup = this->tryGetPropertyGroup(keyStr);

        if (!propertyGroup.has_value()) {
            throw Exception(
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

    const AddressSpace & TargetDescriptionFile::getAddressSpace(std::string_view key) const {
        const auto addressSpace = this->tryGetAddressSpace(key);

        if (!addressSpace.has_value()) {
            throw Exception(
                "Failed to get address space \"" + std::string(key) + "\" from TDF - address space not found"
            );
        }

        return addressSpace->get();
    }

    void TargetDescriptionFile::init(const std::string& xmlFilePath) {
        auto file = QFile(QString::fromStdString(xmlFilePath));
        if (!file.exists()) {
            // This can happen if someone has been messing with the Resources directory.
            throw Exception("Failed to load target description file - file not found");
        }

        file.open(QIODevice::ReadOnly);
        auto document = QDomDocument();
        if (!document.setContent(file.readAll())) {
            throw Exception("Failed to parse target description file - please report this error "
                "to Bloom developers via " + Services::PathService::homeDomainName() + "/report-issue");
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
            this->propertyGroupsMappedByKey.insert(
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

        this->loadModules(document);
        this->loadPeripheralModules(document);
        this->loadVariants(document);
        this->loadPinouts(document);
        this->loadInterfaces(document);
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
            throw Exception(
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
            output.propertiesMappedByKey.insert(std::pair(property.key, std::move(property)));
        }

        for (
            auto element = xmlElement.firstChildElement("property-group");
            !element.isNull();
            element = element.nextSiblingElement("property-group")
        ) {
            auto subgroup = TargetDescriptionFile::propertyGroupFromXml(element);
            output.subgroupsMappedByKey.insert(std::pair(subgroup.key, std::move(subgroup)));
        }

        return output;
    }

    Property TargetDescriptionFile::propertyFromXml(const QDomElement& xmlElement) {
        return Property(
            TargetDescriptionFile::getAttribute(xmlElement, "key"),
            TargetDescriptionFile::getAttribute(xmlElement, "value")
        );
    }

    AddressSpace TargetDescriptionFile::addressSpaceFromXml(const QDomElement &xmlElement) {
        static const auto endiannessByName = BiMap<std::string, TargetMemoryEndianness>({
            {"big", TargetMemoryEndianness::BIG},
            {"little", TargetMemoryEndianness::LITTLE},
        });

        const auto endiannessName = TargetDescriptionFile::tryGetAttribute(xmlElement, "endianness");

        auto endianness = std::optional<TargetMemoryEndianness>();
        if (endiannessName.has_value()) {
            endianness = endiannessByName.valueAt(*endiannessName);

            if (!endianness.has_value()) {
                throw Exception(
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
            throw Exception(
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

    RegisterGroup TargetDescriptionFile::registerGroupFromXml(const QDomElement& xmlElement) {
        if (!xmlElement.hasAttribute("name")) {
            throw Exception("Missing register group name attribute");
        }

        auto registerGroup = RegisterGroup();
        registerGroup.name = xmlElement.attribute("name").toLower().toStdString();

        if (registerGroup.name.empty()) {
            throw Exception("Empty register group name");
        }

        if (xmlElement.hasAttribute("name-in-module")) {
            registerGroup.moduleName = xmlElement.attribute("name-in-module").toLower().toStdString();
        }

        if (xmlElement.hasAttribute("address-space")) {
            registerGroup.addressSpaceId = xmlElement.attribute("address-space").toLower().toStdString();
        }

        if (xmlElement.hasAttribute("offset")) {
            registerGroup.offset = xmlElement.attribute("offset").toInt(nullptr, 16);
        }

        auto& registers = registerGroup.registersMappedByName;
        auto registerNodes = xmlElement.elementsByTagName("register");
        for (int registerIndex = 0; registerIndex < registerNodes.count(); registerIndex++) {
            try {
                auto reg = TargetDescriptionFile::registerFromXml(
                    registerNodes.item(registerIndex).toElement()
                );
                registers.insert(std::pair(reg.name, reg));

            } catch (const Exception& exception) {
                Logger::debug("Failed to extract register from register group target description element - "
                    + exception.getMessage());
            }
        }

        return registerGroup;
    }

    Register TargetDescriptionFile::registerFromXml(const QDomElement& xmlElement) {
        if (
            !xmlElement.hasAttribute("name")
            || !xmlElement.hasAttribute("offset")
            || !xmlElement.hasAttribute("size")
        ) {
            throw Exception("Missing register name/offset/size attribute");
        }

        auto reg = Register();
        reg.name = xmlElement.attribute("name").toLower().toStdString();

        if (reg.name.empty()) {
            throw Exception("Empty register name");
        }

        if (xmlElement.hasAttribute("caption")) {
            reg.caption = xmlElement.attribute("caption").toStdString();
        }

        if (xmlElement.hasAttribute("ocd-rw")) {
            reg.readWriteAccess = xmlElement.attribute("ocd-rw").toLower().toStdString();

        } else if (xmlElement.hasAttribute("rw")) {
            reg.readWriteAccess = xmlElement.attribute("rw").toLower().toStdString();
        }

        bool conversionStatus = false;
        reg.size = xmlElement.attribute("size").toUShort(nullptr, 10);
        reg.offset = xmlElement.attribute("offset").toUShort(&conversionStatus, 16);

        if (!conversionStatus) {
            // Failed to convert offset hex value as string to uint16_t
            throw Exception("Invalid register offset");
        }

        auto& bitFields = reg.bitFieldsMappedByName;
        auto bitFieldNodes = xmlElement.elementsByTagName("bitfield");
        for (int bitFieldIndex = 0; bitFieldIndex < bitFieldNodes.count(); bitFieldIndex++) {
            try {
                auto bitField = TargetDescriptionFile::bitFieldFromXml(
                    bitFieldNodes.item(bitFieldIndex).toElement()
                );
                bitFields.insert(std::pair(bitField.name, bitField));

            } catch (const Exception& exception) {
                Logger::debug("Failed to extract bit field from register target description element - "
                    + exception.getMessage());
            }
        }

        return reg;
    }

    BitField TargetDescriptionFile::bitFieldFromXml(const QDomElement& xmlElement) {
        if (!xmlElement.hasAttribute("name") || !xmlElement.hasAttribute("mask")) {
            throw Exception("Missing bit field name/mask attribute");
        }

        auto bitField = BitField();
        bitField.name = xmlElement.attribute("name").toLower().toStdString();

        auto maskConversion = false;
        bitField.mask = static_cast<std::uint8_t>(
            xmlElement.attribute("mask").toUShort(&maskConversion, 16)
        );

        if (!maskConversion) {
            throw Exception("Failed to convert bit field mask to integer (from hex string)");
        }

        if (bitField.name.empty()) {
            throw Exception("Empty bit field name");
        }

        return bitField;
    }

    const std::string& TargetDescriptionFile::deviceAttribute(const std::string& attributeName) const {
        const auto attributeIt = this->deviceAttributesByName.find(attributeName);

        if (attributeIt == this->deviceAttributesByName.end()) {
            throw Exception("Missing target device attribute (\"" + attributeName + "\")");
        }

        return attributeIt->second;
    }

    void TargetDescriptionFile::loadModules(const QDomDocument& document) {
        const auto deviceElement = document.elementsByTagName("device").item(0).toElement();

        auto moduleNodes = document.elementsByTagName("modules").item(0).toElement()
            .elementsByTagName("module");

        for (int moduleIndex = 0; moduleIndex < moduleNodes.count(); moduleIndex++) {
            auto moduleElement = moduleNodes.item(moduleIndex).toElement();
            auto moduleName = moduleElement.attributes().namedItem("name").nodeValue().toLower().toStdString();
            Module module;
            module.name = moduleName;

            auto registerGroupNodes = moduleElement.elementsByTagName("register-group");
            for (int registerGroupIndex = 0; registerGroupIndex < registerGroupNodes.count(); registerGroupIndex++) {
                auto registerGroup = TargetDescriptionFile::registerGroupFromXml(
                    registerGroupNodes.item(registerGroupIndex).toElement()
                );

                module.registerGroupsMappedByName.insert(std::pair(registerGroup.name, registerGroup));
            }

            this->modulesMappedByName.insert(std::pair(module.name, module));
        }
    }

    void TargetDescriptionFile::loadPeripheralModules(const QDomDocument& document) {
        const auto deviceElement = document.elementsByTagName("device").item(0).toElement();

        auto moduleNodes = deviceElement.elementsByTagName("peripherals").item(0).toElement()
            .elementsByTagName("module");

        for (int moduleIndex = 0; moduleIndex < moduleNodes.count(); moduleIndex++) {
            auto moduleElement = moduleNodes.item(moduleIndex).toElement();
            auto moduleName = moduleElement.attributes().namedItem("name").nodeValue().toLower().toStdString();
            Module module;
            module.name = moduleName;

            auto registerGroupNodes = moduleElement.elementsByTagName("register-group");
            for (int registerGroupIndex = 0; registerGroupIndex < registerGroupNodes.count(); registerGroupIndex++) {
                auto registerGroup = TargetDescriptionFile::registerGroupFromXml(
                    registerGroupNodes.item(registerGroupIndex).toElement()
                );

                module.registerGroupsMappedByName.insert(std::pair(registerGroup.name, registerGroup));

                if (registerGroup.moduleName.has_value()) {
                    this->peripheralRegisterGroupsMappedByModuleRegisterGroupName[registerGroup.moduleName.value()]
                        .emplace_back(registerGroup);
                }
            }

            auto instanceNodes = moduleElement.elementsByTagName("instance");
            for (int instanceIndex = 0; instanceIndex < instanceNodes.count(); instanceIndex++) {
                auto instanceXml = instanceNodes.item(instanceIndex).toElement();
                auto instance = ModuleInstance();
                instance.name = instanceXml.attribute("name").toLower().toStdString();

                auto registerGroupNodes = instanceXml.elementsByTagName("register-group");
                for (
                    int registerGroupIndex = 0;
                    registerGroupIndex < registerGroupNodes.count();
                    registerGroupIndex++
                ) {
                    auto registerGroup = TargetDescriptionFile::registerGroupFromXml(
                        registerGroupNodes.item(registerGroupIndex).toElement()
                    );

                    instance.registerGroupsMappedByName.insert(std::pair(registerGroup.name, registerGroup));
                }

                auto signalNodes = instanceXml.elementsByTagName("signals").item(0).toElement()
                    .elementsByTagName("signal");
                for (int signalIndex = 0; signalIndex < signalNodes.count(); signalIndex++) {
                    auto signalXml = signalNodes.item(signalIndex).toElement();
                    auto signal = Signal();

                    if (!signalXml.hasAttribute("pad")) {
                        continue;
                    }

                    signal.padName = signalXml.attribute("pad").toLower().toStdString();
                    signal.function = signalXml.attribute("function").toStdString();
                    signal.group = signalXml.attribute("group").toStdString();
                    auto indexAttribute = signalXml.attribute("index");
                    bool indexValid = false;
                    auto indexValue = indexAttribute.toInt(&indexValid, 10);

                    if (!indexAttribute.isEmpty() && indexValid) {
                        signal.index = indexValue;
                    }

                    instance.instanceSignals.emplace_back(signal);
                }

                module.instancesMappedByName.insert(std::pair(instance.name, instance));
            }

            this->peripheralModulesMappedByName.insert(std::pair(module.name, module));
        }
    }

    void TargetDescriptionFile::loadVariants(const QDomDocument& document) {
        const auto deviceElement = document.elementsByTagName("device").item(0).toElement();

        auto variantNodes = document.elementsByTagName("variants").item(0).toElement()
            .elementsByTagName("variant");

        for (int variantIndex = 0; variantIndex < variantNodes.count(); variantIndex++) {
            try {
                auto variantXml = variantNodes.item(variantIndex).toElement();

                if (!variantXml.hasAttribute("name")) {
                    throw Exception("Missing name attribute");
                }

                if (!variantXml.hasAttribute("package")) {
                    throw Exception("Missing package attribute");
                }

                if (!variantXml.hasAttribute("pinout")) {
                    throw Exception("Missing pinout attribute");
                }

                auto variant = Variant();
                variant.name = variantXml.attribute("name").toStdString();
                variant.pinoutName = variantXml.attribute("pinout").toLower().toStdString();
                variant.package = variantXml.attribute("package").toUpper().toStdString();

                if (variantXml.hasAttribute("disabled")) {
                    variant.disabled = (variantXml.attribute("disabled") == "1");
                }

                this->variants.push_back(variant);

            } catch (const Exception& exception) {
                Logger::debug(
                    "Failed to extract variant from target description element - " + exception.getMessage()
                );
            }
        }
    }

    void TargetDescriptionFile::loadPinouts(const QDomDocument& document) {
        const auto deviceElement = document.elementsByTagName("device").item(0).toElement();

        auto pinoutNodes = document.elementsByTagName("pinouts").item(0).toElement()
            .elementsByTagName("pinout");

        for (int pinoutIndex = 0; pinoutIndex < pinoutNodes.count(); pinoutIndex++) {
            try {
                auto pinoutXml = pinoutNodes.item(pinoutIndex).toElement();

                if (!pinoutXml.hasAttribute("name")) {
                    throw Exception("Missing name attribute");
                }

                auto pinout = Pinout();
                pinout.name = pinoutXml.attribute("name").toLower().toStdString();

                auto pinNodes = pinoutXml.elementsByTagName("pin");

                for (int pinIndex = 0; pinIndex < pinNodes.count(); pinIndex++) {
                    auto pinXml = pinNodes.item(pinIndex).toElement();

                    if (!pinXml.hasAttribute("position")) {
                        throw Exception(
                            "Missing position attribute on pin element " + std::to_string(pinIndex)
                        );
                    }

                    if (!pinXml.hasAttribute("pad")) {
                        throw Exception("Missing pad attribute on pin element " + std::to_string(pinIndex));
                    }

                    auto pin = Pin();
                    bool positionConversionSucceeded = true;
                    pin.position = pinXml.attribute("position").toInt(&positionConversionSucceeded, 10);
                    pin.pad = pinXml.attribute("pad").toLower().toStdString();

                    if (!positionConversionSucceeded) {
                        throw Exception("Failed to convert position attribute value to integer on pin element "
                            + std::to_string(pinIndex));
                    }

                    pinout.pins.push_back(pin);
                }

                this->pinoutsMappedByName.insert(std::pair(pinout.name, pinout));

            } catch (const Exception& exception) {
                Logger::debug(
                    "Failed to extract pinout from target description element - " + exception.getMessage()
                );
            }
        }
    }

    void TargetDescriptionFile::loadInterfaces(const QDomDocument& document) {
        const auto deviceElement = document.elementsByTagName("device").item(0).toElement();

        auto interfaceNodes = deviceElement.elementsByTagName("interfaces").item(0).toElement()
            .elementsByTagName("interface");

        for (int interfaceIndex = 0; interfaceIndex < interfaceNodes.count(); interfaceIndex++) {
            try {
                auto interfaceXml = interfaceNodes.item(interfaceIndex).toElement();

                if (!interfaceXml.hasAttribute("name")) {
                    throw Exception("Missing name attribute");
                }

                auto interface = Interface();
                interface.name = interfaceXml.attribute("name").toLower().toStdString();

                if (interfaceXml.hasAttribute("type")) {
                    interface.type = interfaceXml.attribute("type").toStdString();
                }

                this->interfacesByName.insert(std::pair(interface.name, interface));

            } catch (const Exception& exception) {
                Logger::debug(
                    "Failed to extract interface from target description element - " + exception.getMessage()
                );
            }
        }
    }
}
