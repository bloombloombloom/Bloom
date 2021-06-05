#include <QJsonDocument>
#include <QJsonArray>

#include "TargetDescriptionFile.hpp"
#include "Exceptions/TargetDescriptionParsingFailureException.hpp"
#include "src/Logger/Logger.hpp"

using namespace Bloom::Targets::TargetDescription;
using namespace Bloom::Exceptions;

void TargetDescriptionFile::init(const QString& xmlFilePath) {
    auto file = QFile(xmlFilePath);
    if (!file.exists()) {
        // This can happen if someone has been messing with the Resources directory.
        throw Exception("Failed to load target description file - file not found");
    }

    file.open(QIODevice::ReadOnly);
    auto xml = QDomDocument();
    xml.setContent(file.readAll());
    this->init(xml);
}

void TargetDescriptionFile::init(const QDomDocument& xml) {
    this->xml = xml;

    auto device = xml.elementsByTagName("device").item(0).toElement();

    if (!device.isElement()) {
        throw TargetDescriptionParsingFailureException("Device element not found.");
    }

    this->deviceElement = device;
}

std::string TargetDescriptionFile::getTargetName() const {
    return this->deviceElement.attributes().namedItem("name").nodeValue().toStdString();
}

AddressSpace TargetDescriptionFile::generateAddressSpaceFromXml(const QDomElement& xmlElement) const {
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
    addressSpace.startAddress = xmlElement.attribute("start").toUShort(&conversionStatus, 16);

    if (!conversionStatus) {
        throw Exception("Failed to convert start address hex value to integer.");
    }

    addressSpace.size = xmlElement.attribute("size").toUShort(&conversionStatus, 16);

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
            auto segment = this->generateMemorySegmentFromXml(segmentNodes.item(segmentIndex).toElement());

            if (memorySegments.find(segment.type) == memorySegments.end()) {
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

MemorySegment TargetDescriptionFile::generateMemorySegmentFromXml(const QDomElement& xmlElement) const {
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

RegisterGroup TargetDescriptionFile::generateRegisterGroupFromXml(const QDomElement& xmlElement) const {
    if (!xmlElement.hasAttribute("name")) {
        throw Exception("Missing register group name attribute");
    }

    auto registerGroup = RegisterGroup();
    registerGroup.name = xmlElement.attribute("name").toLower().toStdString();

    if (registerGroup.name.empty()) {
        throw Exception("Empty register group name");
    }

    if (xmlElement.hasAttribute("offset")) {
        registerGroup.offset = xmlElement.attribute("offset").toInt(nullptr, 16);
    }

    auto& registers = registerGroup.registersMappedByName;
    auto registerNodes = xmlElement.elementsByTagName("register");
    for (int registerIndex = 0; registerIndex < registerNodes.count(); registerIndex++) {
        try {
            auto reg = this->generateRegisterFromXml(registerNodes.item(registerIndex).toElement());
            registers.insert(std::pair(reg.name, reg));

        } catch (const Exception& exception) {
            Logger::debug("Failed to extract register from register group target description element - "
                + exception.getMessage());
        }
    }

    return registerGroup;
}

Register TargetDescriptionFile::generateRegisterFromXml(const QDomElement& xmlElement) const {
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

    bool conversionStatus;
    reg.size = xmlElement.attribute("size").toUShort(nullptr, 10);
    reg.offset = xmlElement.attribute("offset").toUShort(&conversionStatus, 16);

    if (!conversionStatus) {
        // Failed to convert offset hex value as string to uint16_t
        throw Exception("Invalid register offset");
    }

    return reg;
}

const std::map<std::string, PropertyGroup>& TargetDescriptionFile::getPropertyGroupsMappedByName() const {
    if (!this->cachedPropertyGroupMapping.has_value()) {
        if (!this->deviceElement.isElement()) {
            throw TargetDescriptionParsingFailureException("Device element not found.");
        }

        std::map<std::string, PropertyGroup> output;
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

            output.insert(std::pair(propertyGroup.name, propertyGroup));
        }

        this->cachedPropertyGroupMapping.emplace(output);
    }

    return this->cachedPropertyGroupMapping.value();
}

const std::map<std::string, Module>& TargetDescriptionFile::getModulesMappedByName() const {
    if (!this->cachedModuleByNameMapping.has_value()) {
        std::map<std::string, Module> output;
        auto moduleNodes = this->xml.elementsByTagName("modules").item(0).toElement()
            .elementsByTagName("module");

        for (int moduleIndex = 0; moduleIndex < moduleNodes.count(); moduleIndex++) {
            auto moduleElement = moduleNodes.item(moduleIndex).toElement();
            auto moduleName = moduleElement.attributes().namedItem("name").nodeValue().toLower().toStdString();
            Module module;
            module.name = moduleName;

            auto registerGroupNodes = moduleElement.elementsByTagName("register-group");
            for (int registerGroupIndex = 0; registerGroupIndex < registerGroupNodes.count(); registerGroupIndex++) {
                auto registerGroup = this->generateRegisterGroupFromXml(registerGroupNodes.item(registerGroupIndex).toElement());

                module.registerGroupsMappedByName.insert(std::pair(registerGroup.name, registerGroup));
            }

            output.insert(std::pair(module.name, module));
        }

        this->cachedModuleByNameMapping.emplace(output);
    }

    return this->cachedModuleByNameMapping.value();
}

const std::map<std::string, Module>& TargetDescriptionFile::getPeripheralModulesMappedByName() const {
    if (!this->cachedPeripheralModuleByNameMapping.has_value()) {
        std::map<std::string, Module> output;
        auto moduleNodes = this->deviceElement.elementsByTagName("peripherals").item(0).toElement()
            .elementsByTagName("module");

        for (int moduleIndex = 0; moduleIndex < moduleNodes.count(); moduleIndex++) {
            auto moduleElement = moduleNodes.item(moduleIndex).toElement();
            auto moduleName = moduleElement.attributes().namedItem("name").nodeValue().toLower().toStdString();
            Module module;
            module.name = moduleName;

            auto registerGroupNodes = moduleElement.elementsByTagName("register-group");
            for (int registerGroupIndex = 0; registerGroupIndex < registerGroupNodes.count(); registerGroupIndex++) {
                auto registerGroup = this->generateRegisterGroupFromXml(registerGroupNodes.item(registerGroupIndex).toElement());

                module.registerGroupsMappedByName.insert(std::pair(registerGroup.name, registerGroup));
            }

            auto instanceNodes = moduleElement.elementsByTagName("instance");
            for (int instanceIndex = 0; instanceIndex < instanceNodes.count(); instanceIndex++) {
                auto instanceXml = instanceNodes.item(instanceIndex).toElement();
                auto instance = ModuleInstance();
                instance.name = instanceXml.attribute("name").toLower().toStdString();

                auto registerGroupNodes = instanceXml.elementsByTagName("register-group");
                for (int registerGroupIndex = 0; registerGroupIndex < registerGroupNodes.count(); registerGroupIndex++) {
                    auto registerGroup = this->generateRegisterGroupFromXml(registerGroupNodes.item(registerGroupIndex).toElement());

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

            output.insert(std::pair(module.name, module));
        }

        this->cachedPeripheralModuleByNameMapping.emplace(output);
    }

    return this->cachedPeripheralModuleByNameMapping.value();
}

std::map<std::string, AddressSpace> TargetDescriptionFile::getAddressSpacesMappedById() const {
    std::map<std::string, AddressSpace> output;

    auto addressSpaceNodes = this->deviceElement.elementsByTagName("address-spaces").item(0).toElement()
        .elementsByTagName("address-space");

    for (int addressSpaceIndex = 0; addressSpaceIndex < addressSpaceNodes.count(); addressSpaceIndex++) {
        try {
            auto addressSpace = this->generateAddressSpaceFromXml(addressSpaceNodes.item(addressSpaceIndex).toElement());
            output.insert(std::pair(addressSpace.id, addressSpace));

        } catch (const Exception& exception) {
            Logger::debug("Failed to extract address space from target description element - " + exception.getMessage());
        }
    }

    return output;
}

std::optional<MemorySegment> TargetDescriptionFile::getFlashMemorySegment() const {
    auto addressMapping = this->getAddressSpacesMappedById();
    auto programAddressSpaceIt = addressMapping.find("prog");

    // Flash memory attributes are typically found in memory segments within the program address space.
    if (programAddressSpaceIt != addressMapping.end()) {
        auto& programAddressSpace = programAddressSpaceIt->second;
        auto& programMemorySegments = programAddressSpace.memorySegmentsByTypeAndName;

        if (programMemorySegments.find(MemorySegmentType::FLASH) != programMemorySegments.end()) {
            auto& flashMemorySegments = programMemorySegments.find(MemorySegmentType::FLASH)->second;

            /*
             * Some target descriptions describe the flash memory segments in the "APP_SECTION" segment, whereas
             * others use the "FLASH" segment.
             */
            auto flashSegmentIt = flashMemorySegments.find("app_section") != flashMemorySegments.end() ?
                                  flashMemorySegments.find("app_section") : flashMemorySegments.find("flash");

            if (flashSegmentIt != flashMemorySegments.end()) {
                return flashSegmentIt->second;
            }
        }
    }

    return std::nullopt;
}

std::optional<MemorySegment> TargetDescriptionFile::getRamMemorySegment() const {
    auto addressMapping = this->getAddressSpacesMappedById();

    // Internal RAM  &register attributes are usually found in the data address space
    auto dataAddressSpaceIt = addressMapping.find("data");

    if (dataAddressSpaceIt != addressMapping.end()) {
        auto& dataAddressSpace = dataAddressSpaceIt->second;
        auto& dataMemorySegments = dataAddressSpace.memorySegmentsByTypeAndName;

        if (dataMemorySegments.find(MemorySegmentType::RAM) != dataMemorySegments.end()) {
            auto& ramMemorySegments = dataMemorySegments.find(MemorySegmentType::RAM)->second;
            auto ramMemorySegmentIt = ramMemorySegments.begin();

            if (ramMemorySegmentIt != ramMemorySegments.end()) {
                return ramMemorySegmentIt->second;
            }
        }
    }

    return std::nullopt;
}

std::optional<MemorySegment> TargetDescriptionFile::getRegisterMemorySegment() const {
    auto addressMapping = this->getAddressSpacesMappedById();

    // Internal RAM  &register attributes are usually found in the data address space
    auto dataAddressSpaceIt = addressMapping.find("data");

    if (dataAddressSpaceIt != addressMapping.end()) {
        auto& dataAddressSpace = dataAddressSpaceIt->second;
        auto& dataMemorySegments = dataAddressSpace.memorySegmentsByTypeAndName;

        if (dataMemorySegments.find(MemorySegmentType::REGISTERS) != dataMemorySegments.end()) {
            auto& registerMemorySegments = dataMemorySegments.find(MemorySegmentType::REGISTERS)->second;
            auto registerMemorySegmentIt = registerMemorySegments.begin();

            if (registerMemorySegmentIt != registerMemorySegments.end()) {
                return registerMemorySegmentIt->second;
            }
        }
    }

    return std::nullopt;
}

std::optional<MemorySegment> TargetDescriptionFile::getEepromMemorySegment() const {
    auto addressMapping = this->getAddressSpacesMappedById();

    // EEPROM attributes are usually found in the data address space
    auto eepromAddressSpaceIt = addressMapping.find("eeprom");

    if (eepromAddressSpaceIt != addressMapping.end()) {
        auto& eepromAddressSpace = eepromAddressSpaceIt->second;
        auto& eepromAddressSpaceSegments = eepromAddressSpace.memorySegmentsByTypeAndName;

        if (eepromAddressSpaceSegments.find(MemorySegmentType::EEPROM) != eepromAddressSpaceSegments.end()) {
            auto& eepromMemorySegments = eepromAddressSpaceSegments.find(MemorySegmentType::EEPROM)->second;
            auto eepromMemorySegmentIt = eepromMemorySegments.begin();

            if (eepromMemorySegmentIt != eepromMemorySegments.end()) {
                return eepromMemorySegmentIt->second;
            }
        }
    }

    return std::nullopt;
}

std::optional<MemorySegment> TargetDescriptionFile::getFirstBootSectionMemorySegment() const {
    auto addressMapping = this->getAddressSpacesMappedById();
    auto programAddressSpaceIt = addressMapping.find("prog");

    if (programAddressSpaceIt != addressMapping.end()) {
        auto& programAddressSpace = programAddressSpaceIt->second;
        auto& programMemorySegments = programAddressSpace.memorySegmentsByTypeAndName;

        if (programMemorySegments.find(MemorySegmentType::FLASH) != programMemorySegments.end()) {
            auto& flashMemorySegments = programMemorySegments.find(MemorySegmentType::FLASH)->second;

            if (flashMemorySegments.contains("boot_section_1")) {
                return flashMemorySegments.at("boot_section_1");

            } else if (flashMemorySegments.contains("boot_section")) {
                return flashMemorySegments.at("boot_section");
            }
        }
    }

    return std::nullopt;
}

std::optional<RegisterGroup> TargetDescriptionFile::getCpuRegisterGroup() const {
    auto& modulesByName = this->getModulesMappedByName();

    if (modulesByName.find("cpu") != modulesByName.end()) {
        auto cpuModule = modulesByName.find("cpu")->second;
        auto cpuRegisterGroupIt = cpuModule.registerGroupsMappedByName.find("cpu");

        if (cpuRegisterGroupIt != cpuModule.registerGroupsMappedByName.end()) {
            return cpuRegisterGroupIt->second;
        }
    }

    return std::nullopt;
}

std::optional<RegisterGroup> TargetDescriptionFile::getBootLoadRegisterGroup() const {
    auto& modulesByName = this->getModulesMappedByName();

    if (modulesByName.contains("boot_load")) {
        auto& bootLoadModule = modulesByName.at("boot_load");
        auto bootLoadRegisterGroupIt = bootLoadModule.registerGroupsMappedByName.find("boot_load");

        if (bootLoadRegisterGroupIt != bootLoadModule.registerGroupsMappedByName.end()) {
            return bootLoadRegisterGroupIt->second;
        }
    }

    return std::nullopt;
}

std::optional<RegisterGroup> TargetDescriptionFile::getEepromRegisterGroup() const {
    auto& modulesByName = this->getModulesMappedByName();

    if (modulesByName.find("eeprom") != modulesByName.end()) {
        auto eepromModule = modulesByName.find("eeprom")->second;
        auto eepromRegisterGroupIt = eepromModule.registerGroupsMappedByName.find("eeprom");

        if (eepromRegisterGroupIt != eepromModule.registerGroupsMappedByName.end()) {
            return eepromRegisterGroupIt->second;
        }
    }

    return std::nullopt;
}

std::optional<Register> TargetDescriptionFile::getStatusRegister() const {
    auto cpuRegisterGroup = this->getCpuRegisterGroup();

    if (cpuRegisterGroup.has_value()) {
        auto statusRegisterIt = cpuRegisterGroup->registersMappedByName.find("sreg");

        if (statusRegisterIt != cpuRegisterGroup->registersMappedByName.end()) {
            return statusRegisterIt->second;
        }
    }

    return std::nullopt;
}

std::optional<Register> TargetDescriptionFile::getStackPointerRegister() const {
    auto cpuRegisterGroup = this->getCpuRegisterGroup();

    if (cpuRegisterGroup.has_value()) {
        auto stackPointerRegisterIt = cpuRegisterGroup->registersMappedByName.find("sp");

        if (stackPointerRegisterIt != cpuRegisterGroup->registersMappedByName.end()) {
            return stackPointerRegisterIt->second;
        }
    }

    return std::nullopt;
}

std::optional<Register> TargetDescriptionFile::getStackPointerHighRegister() const {
    auto cpuRegisterGroup = this->getCpuRegisterGroup();

    if (cpuRegisterGroup.has_value()) {
        auto stackPointerHighRegisterIt = cpuRegisterGroup->registersMappedByName.find("sph");

        if (stackPointerHighRegisterIt != cpuRegisterGroup->registersMappedByName.end()) {
            return stackPointerHighRegisterIt->second;
        }
    }

    return std::nullopt;
}

std::optional<Register> TargetDescriptionFile::getStackPointerLowRegister() const {
    auto cpuRegisterGroup = this->getCpuRegisterGroup();

    if (cpuRegisterGroup.has_value()) {
        auto stackPointerLowRegisterIt = cpuRegisterGroup->registersMappedByName.find("spl");

        if (stackPointerLowRegisterIt != cpuRegisterGroup->registersMappedByName.end()) {
            return stackPointerLowRegisterIt->second;
        }
    }

    return std::nullopt;
}

std::optional<Register> TargetDescriptionFile::getOscillatorCalibrationRegister() const {
    auto cpuRegisterGroup = this->getCpuRegisterGroup();

    if (cpuRegisterGroup.has_value()) {
        auto osccalRegisterIt = cpuRegisterGroup->registersMappedByName.find("osccal");

        if (osccalRegisterIt != cpuRegisterGroup->registersMappedByName.end()) {
            return osccalRegisterIt->second;
        }
    }

    return std::nullopt;
}

std::optional<Register> TargetDescriptionFile::getSpmcsRegister() const {
    auto cpuRegisterGroup = this->getCpuRegisterGroup();

    if (cpuRegisterGroup.has_value() && cpuRegisterGroup->registersMappedByName.contains("spmcsr")) {
        return cpuRegisterGroup->registersMappedByName.at("spmcsr");

    } else {
        auto bootLoadRegisterGroup = this->getBootLoadRegisterGroup();

        if (bootLoadRegisterGroup.has_value() && bootLoadRegisterGroup->registersMappedByName.contains("spmcsr")) {
            return bootLoadRegisterGroup->registersMappedByName.at("spmcsr");
        }
    }

    return std::nullopt;
}

std::optional<Register> TargetDescriptionFile::getSpmcRegister() const {
    auto bootLoadRegisterGroup = this->getBootLoadRegisterGroup();

    if (bootLoadRegisterGroup.has_value() && bootLoadRegisterGroup->registersMappedByName.contains("spmcr")) {
        return bootLoadRegisterGroup->registersMappedByName.at("spmcr");

    } else {
        auto cpuRegisterGroup = this->getCpuRegisterGroup();

        if (cpuRegisterGroup.has_value() && cpuRegisterGroup->registersMappedByName.contains("spmcr")) {
            return cpuRegisterGroup->registersMappedByName.at("spmcr");
        }
    }

    return std::nullopt;
}

std::optional<Register> TargetDescriptionFile::getEepromAddressRegister() const {
    auto eepromRegisterGroup = this->getEepromRegisterGroup();

    if (eepromRegisterGroup.has_value()) {
        auto eepromAddressRegisterIt = eepromRegisterGroup->registersMappedByName.find("eear");

        if (eepromAddressRegisterIt != eepromRegisterGroup->registersMappedByName.end()) {
            return eepromAddressRegisterIt->second;
        }
    }

    return std::nullopt;
}

std::optional<Register> TargetDescriptionFile::getEepromAddressLowRegister() const {
    auto eepromRegisterGroup = this->getEepromRegisterGroup();

    if (eepromRegisterGroup.has_value()) {
        auto eepromAddressRegisterIt = eepromRegisterGroup->registersMappedByName.find("eearl");

        if (eepromAddressRegisterIt != eepromRegisterGroup->registersMappedByName.end()) {
            return eepromAddressRegisterIt->second;
        }
    }

    return std::nullopt;
}

std::optional<Register> TargetDescriptionFile::getEepromAddressHighRegister() const {
    auto eepromRegisterGroup = this->getEepromRegisterGroup();

    if (eepromRegisterGroup.has_value()) {
        auto eepromAddressRegisterIt = eepromRegisterGroup->registersMappedByName.find("eearh");

        if (eepromAddressRegisterIt != eepromRegisterGroup->registersMappedByName.end()) {
            return eepromAddressRegisterIt->second;
        }
    }

    return std::nullopt;
}

std::optional<Register> TargetDescriptionFile::getEepromDataRegister() const {
    auto eepromRegisterGroup = this->getEepromRegisterGroup();

    if (eepromRegisterGroup.has_value()) {
        auto eepromDataRegisterIt = eepromRegisterGroup->registersMappedByName.find("eedr");

        if (eepromDataRegisterIt != eepromRegisterGroup->registersMappedByName.end()) {
            return eepromDataRegisterIt->second;
        }
    }

    return std::nullopt;
}

std::optional<Register> TargetDescriptionFile::getEepromControlRegister() const {
    auto eepromRegisterGroup = this->getEepromRegisterGroup();

    if (eepromRegisterGroup.has_value()) {
        auto eepromControlRegisterIt = eepromRegisterGroup->registersMappedByName.find("eecr");

        if (eepromControlRegisterIt != eepromRegisterGroup->registersMappedByName.end()) {
            return eepromControlRegisterIt->second;
        }
    }

    return std::nullopt;
}

std::vector<Variant> TargetDescriptionFile::getVariants() const {
    std::vector<Variant> output;

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
            variant.orderCode = variantXml.attribute("ordercode").toStdString();
            variant.pinoutName = variantXml.attribute("pinout").toLower().toStdString();
            variant.package = variantXml.attribute("package").toUpper().toStdString();

            if (variantXml.hasAttribute("disabled")) {
                variant.disabled = (variantXml.attribute("disabled") == "1");
            }

            output.push_back(variant);

        } catch (const Exception& exception) {
            Logger::debug("Failed to extract variant from target description element - " + exception.getMessage());
        }
    }

    return output;
}

const std::map<std::string, Pinout>& TargetDescriptionFile::getPinoutsMappedByName() const {
    if (!this->cachedPinoutByNameMapping.has_value()) {
        this->cachedPinoutByNameMapping = std::map<std::string, Pinout>();

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

                this->cachedPinoutByNameMapping->insert(std::pair(pinout.name, pinout));

            } catch (const Exception& exception) {
                Logger::debug("Failed to extract pinout from target description element - " + exception.getMessage());
            }
        }
    }

    return this->cachedPinoutByNameMapping.value();
}
