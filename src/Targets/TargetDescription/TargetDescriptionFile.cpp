#include "TargetDescriptionFile.hpp"

#include <QJsonDocument>
#include <QJsonArray>

#include "Exceptions/TargetDescriptionParsingFailureException.hpp"
#include "src/Logger/Logger.hpp"

using namespace Bloom::Targets::TargetDescription;
using namespace Bloom::Exceptions;

std::string TargetDescriptionFile::getTargetName() const {
    return this->deviceElement.attributes().namedItem("name").nodeValue().toStdString();
}

void TargetDescriptionFile::init(const QString& xmlFilePath) {
    auto file = QFile(xmlFilePath);
    if (!file.exists()) {
        // This can happen if someone has been messing with the Resources directory.
        throw Exception("Failed to load target description file - file not found");
    }

    file.open(QIODevice::ReadOnly);
    auto xml = QDomDocument();
    if (!xml.setContent(file.readAll())) {
        throw Exception("Failed to parse target description file - please report this error "
            "to Bloom developers via https://bloom.oscillate.io/report-issue");
    }

    this->init(xml);
}

void TargetDescriptionFile::init(const QDomDocument& xml) {
    this->xml = xml;

    auto device = xml.elementsByTagName("device").item(0).toElement();
    if (!device.isElement()) {
        throw TargetDescriptionParsingFailureException("Device element not found.");
    }

    this->deviceElement = device;

    this->loadAddressSpaces();
    this->loadPropertyGroups();
    this->loadModules();
    this->loadPeripheralModules();
    this->loadVariants();
    this->loadPinouts();
    this->loadInterfaces();
}

AddressSpace TargetDescriptionFile::generateAddressSpaceFromXml(const QDomElement& xmlElement) {
    if (
        !xmlElement.hasAttribute("id")
        || !xmlElement.hasAttribute("name")
        || !xmlElement.hasAttribute("size")
        || !xmlElement.hasAttribute("start")
    ) {
        throw Exception("Address space element missing id/name/size/start attributes.");
    }

    auto addressSpace = AddressSpace();
    addressSpace.name = xmlElement.attribute("name").toStdString();
    addressSpace.id = xmlElement.attribute("id").toStdString();

    bool conversionStatus;
    addressSpace.startAddress = xmlElement.attribute("start").toUInt(&conversionStatus, 16);

    if (!conversionStatus) {
        throw Exception("Failed to convert start address hex value to integer.");
    }

    addressSpace.size = xmlElement.attribute("size").toUInt(&conversionStatus, 16);

    if (!conversionStatus) {
        throw Exception("Failed to convert size hex value to integer.");
    }

    if (xmlElement.hasAttribute("endianness")) {
        addressSpace.littleEndian = (xmlElement.attribute("endianness").toStdString() == "little");
    }

    // Create memory segment objects and add them to the mapping.
    auto segmentNodes = xmlElement.elementsByTagName("memory-segment");
    auto& memorySegments = addressSpace.memorySegmentsByTypeAndName;
    for (int segmentIndex = 0; segmentIndex < segmentNodes.count(); segmentIndex++) {
        try {
            auto segment = TargetDescriptionFile::generateMemorySegmentFromXml(
                segmentNodes.item(segmentIndex).toElement()
            );

            if (!memorySegments.contains(segment.type)) {
                memorySegments.insert(
                    std::pair<
                        MemorySegmentType,
                        decltype(addressSpace.memorySegmentsByTypeAndName)::mapped_type
                    >(segment.type, {}));
            }

            memorySegments.find(segment.type)->second.insert(std::pair(segment.name, segment));

        } catch (const Exception& exception) {
            Logger::debug("Failed to extract memory segment from target description element - "
                + exception.getMessage());
        }
    }

    return addressSpace;
}

MemorySegment TargetDescriptionFile::generateMemorySegmentFromXml(const QDomElement& xmlElement) {
    if (
        !xmlElement.hasAttribute("type")
        || !xmlElement.hasAttribute("name")
        || !xmlElement.hasAttribute("size")
        || !xmlElement.hasAttribute("start")
    ) {
        throw Exception("Missing type/name/size/start attributes");
    }

    auto segment = MemorySegment();
    auto typeName = xmlElement.attribute("type").toStdString();
    auto type = MemorySegment::typesMappedByName.valueAt(typeName);

    if (!type.has_value()) {
        throw Exception("Unknown type: \"" + typeName + "\"");
    }

    segment.type = type.value();
    segment.name = xmlElement.attribute("name").toLower().toStdString();

    bool conversionStatus;
    segment.startAddress = xmlElement.attribute("start").toUInt(&conversionStatus, 16);

    if (!conversionStatus) {
        // Failed to convert startAddress hex value as string to uint16_t
        throw Exception("Invalid start address");
    }

    segment.size = xmlElement.attribute("size").toUInt(&conversionStatus, 16);

    if (!conversionStatus) {
        // Failed to convert size hex value as string to uint16_t
        throw Exception("Invalid size");
    }

    if (xmlElement.hasAttribute("pagesize")) {
        // The page size can be in single byte hexadecimal form ("0x01"), or it can be in plain integer form!
        auto pageSize = xmlElement.attribute("pagesize");
        segment.pageSize = pageSize.toUInt(&conversionStatus, pageSize.contains("0x") ? 16 : 10);

        if (!conversionStatus) {
            // Failed to convert size hex value as string to uint16_t
            throw Exception("Invalid size");
        }
    }

    return segment;
}

RegisterGroup TargetDescriptionFile::generateRegisterGroupFromXml(const QDomElement& xmlElement) {
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
            auto reg = TargetDescriptionFile::generateRegisterFromXml(
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

Register TargetDescriptionFile::generateRegisterFromXml(const QDomElement& xmlElement) {
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

    bool conversionStatus;
    reg.size = xmlElement.attribute("size").toUShort(nullptr, 10);
    reg.offset = xmlElement.attribute("offset").toUShort(&conversionStatus, 16);

    if (!conversionStatus) {
        // Failed to convert offset hex value as string to uint16_t
        throw Exception("Invalid register offset");
    }

    return reg;
}

void TargetDescriptionFile::loadAddressSpaces() {

    auto addressSpaceNodes = this->deviceElement.elementsByTagName("address-spaces").item(0).toElement()
        .elementsByTagName("address-space");

    for (int addressSpaceIndex = 0; addressSpaceIndex < addressSpaceNodes.count(); addressSpaceIndex++) {
        try {
            auto addressSpace = TargetDescriptionFile::generateAddressSpaceFromXml(
                addressSpaceNodes.item(addressSpaceIndex).toElement()
            );
            this->addressSpacesMappedById.insert(std::pair(addressSpace.id, addressSpace));

        } catch (const Exception& exception) {
            Logger::debug("Failed to extract address space from target description element - " + exception.getMessage());
        }
    }
}

void TargetDescriptionFile::loadPropertyGroups() {
    if (!this->deviceElement.isElement()) {
        throw TargetDescriptionParsingFailureException("Device element not found.");
    }

    auto propertyGroupNodes = this->deviceElement.elementsByTagName("property-groups").item(0).toElement()
        .elementsByTagName("property-group");

    for (int propertyGroupIndex = 0; propertyGroupIndex < propertyGroupNodes.count(); propertyGroupIndex++) {
        auto propertyGroupElement = propertyGroupNodes.item(propertyGroupIndex).toElement();
        auto propertyGroupName = propertyGroupElement.attributes().namedItem("name").nodeValue().toLower().toStdString();
        PropertyGroup propertyGroup;
        propertyGroup.name = propertyGroupName;

        auto propertyNodes = propertyGroupElement.elementsByTagName("property");
        for (int propertyIndex = 0; propertyIndex < propertyNodes.count(); propertyIndex++) {
            auto propertyElement = propertyNodes.item(propertyIndex).toElement();
            auto propertyName = propertyElement.attributes().namedItem("name").nodeValue();

            Property property;
            property.name = propertyName.toStdString();
            property.value = propertyElement.attributes().namedItem("value").nodeValue();

            propertyGroup.propertiesMappedByName.insert(std::pair(propertyName.toLower().toStdString(), property));
        }

        this->propertyGroupsMappedByName.insert(std::pair(propertyGroup.name, propertyGroup));
    }
}

void TargetDescriptionFile::loadModules() {
    auto moduleNodes = this->xml.elementsByTagName("modules").item(0).toElement()
        .elementsByTagName("module");

    for (int moduleIndex = 0; moduleIndex < moduleNodes.count(); moduleIndex++) {
        auto moduleElement = moduleNodes.item(moduleIndex).toElement();
        auto moduleName = moduleElement.attributes().namedItem("name").nodeValue().toLower().toStdString();
        Module module;
        module.name = moduleName;

        auto registerGroupNodes = moduleElement.elementsByTagName("register-group");
        for (int registerGroupIndex = 0; registerGroupIndex < registerGroupNodes.count(); registerGroupIndex++) {
            auto registerGroup = TargetDescriptionFile::generateRegisterGroupFromXml(
                registerGroupNodes.item(registerGroupIndex).toElement()
            );

            module.registerGroupsMappedByName.insert(std::pair(registerGroup.name, registerGroup));
        }

        this->modulesMappedByName.insert(std::pair(module.name, module));
    }
}

void TargetDescriptionFile::loadPeripheralModules() {
    auto moduleNodes = this->deviceElement.elementsByTagName("peripherals").item(0).toElement()
        .elementsByTagName("module");

    for (int moduleIndex = 0; moduleIndex < moduleNodes.count(); moduleIndex++) {
        auto moduleElement = moduleNodes.item(moduleIndex).toElement();
        auto moduleName = moduleElement.attributes().namedItem("name").nodeValue().toLower().toStdString();
        Module module;
        module.name = moduleName;

        auto registerGroupNodes = moduleElement.elementsByTagName("register-group");
        for (int registerGroupIndex = 0; registerGroupIndex < registerGroupNodes.count(); registerGroupIndex++) {
            auto registerGroup = TargetDescriptionFile::generateRegisterGroupFromXml(
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
            for (int registerGroupIndex = 0; registerGroupIndex < registerGroupNodes.count(); registerGroupIndex++) {
                auto registerGroup = TargetDescriptionFile::generateRegisterGroupFromXml(
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

void TargetDescriptionFile::loadVariants() {
    auto variantNodes = this->xml.elementsByTagName("variants").item(0).toElement()
        .elementsByTagName("variant");

    for (int variantIndex = 0; variantIndex < variantNodes.count(); variantIndex++) {
        try {
            auto variantXml = variantNodes.item(variantIndex).toElement();

            if (!variantXml.hasAttribute("ordercode")) {
                throw Exception("Missing ordercode attribute");
            }

            if (!variantXml.hasAttribute("package")) {
                throw Exception("Missing package attribute");
            }

            if (!variantXml.hasAttribute("pinout")) {
                throw Exception("Missing pinout attribute");
            }

            auto variant = Variant();
            variant.name = variantXml.attribute("ordercode").toStdString();
            variant.pinoutName = variantXml.attribute("pinout").toLower().toStdString();
            variant.package = variantXml.attribute("package").toUpper().toStdString();

            if (variantXml.hasAttribute("disabled")) {
                variant.disabled = (variantXml.attribute("disabled") == "1");
            }

            this->variants.push_back(variant);

        } catch (const Exception& exception) {
            Logger::debug("Failed to extract variant from target description element - " + exception.getMessage());
        }
    }
}

void TargetDescriptionFile::loadPinouts() {
    auto pinoutNodes = this->xml.elementsByTagName("pinouts").item(0).toElement()
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
                    throw Exception("Missing position attribute on pin element " + std::to_string(pinIndex));
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
            Logger::debug("Failed to extract pinout from target description element - " + exception.getMessage());
        }
    }
}

void TargetDescriptionFile::loadInterfaces() {
    auto interfaceNodes = this->deviceElement.elementsByTagName("interfaces").item(0).toElement()
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
            Logger::debug("Failed to extract interface from target description element - " + exception.getMessage());
        }
    }
}
