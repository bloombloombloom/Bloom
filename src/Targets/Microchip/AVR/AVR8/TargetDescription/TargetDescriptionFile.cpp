#include <QJsonDocument>
#include <QJsonArray>

#include "TargetDescriptionFile.hpp"
#include "src/Exceptions/Exception.hpp"
#include "src/Targets/TargetDescription/Exceptions/TargetDescriptionParsingFailureException.hpp"
#include "src/Logger/Logger.hpp"
#include "src/Helpers/Paths.hpp"

using namespace Bloom::Targets::Microchip::Avr::Avr8Bit::TargetDescription;
using namespace Bloom::Targets::Microchip::Avr::Avr8Bit;
using namespace Bloom::Targets::Microchip::Avr;
using namespace Bloom::Exceptions;

using Bloom::Targets::TargetDescription::RegisterGroup;
using Bloom::Targets::TargetDescription::MemorySegment;
using Bloom::Targets::TargetDescription::MemorySegmentType;
using Bloom::Targets::TargetDescription::Register;
using Bloom::Targets::TargetVariant;

TargetDescriptionFile::TargetDescriptionFile(
    const TargetSignature& targetSignature,
    std::optional<std::string> targetName
) {
    auto targetSignatureHex = targetSignature.toHex();
    auto mapping = TargetDescriptionFile::getTargetDescriptionMapping();
    auto qTargetSignatureHex = QString::fromStdString(targetSignatureHex).toLower();

    if (mapping.contains(qTargetSignatureHex)) {
        // We have a match for the target signature.
        auto descriptionFilesJsonArray = mapping.find(qTargetSignatureHex).value().toArray();
        auto matchingDescriptionFiles = std::vector<QJsonValue>();
        std::copy_if(
            descriptionFilesJsonArray.begin(),
            descriptionFilesJsonArray.end(),
            std::back_inserter(matchingDescriptionFiles),
            [&targetName] (const QJsonValue& value) {
                auto pdTargetName = value.toObject().find("targetName")->toString().toLower().toStdString();
                return !targetName.has_value() || (targetName.has_value() && targetName.value() == pdTargetName);
            }
        );

        if (targetName.has_value() && matchingDescriptionFiles.empty()) {
            throw Exception("Failed to resolve target description file for target \"" + targetName.value()
                + "\" - target signature \"" + targetSignatureHex + "\" does not belong to target with name \"" +
                targetName.value() + "\". Please review your bloom.json configuration.");
        }

        if (matchingDescriptionFiles.size() == 1) {
            // Attempt to load the XML target description file
            auto descriptionFilePath = QString::fromStdString(Paths::applicationDirPath()) + "/"
                + matchingDescriptionFiles.front().toObject().find("targetDescriptionFilePath")->toString();

            Logger::debug("Loading AVR8 target description file: " + descriptionFilePath.toStdString());
            Targets::TargetDescription::TargetDescriptionFile::init(descriptionFilePath);

        } else if (matchingDescriptionFiles.size() > 1) {
            /*
             * There are numerous target description files mapped to this target signature. There's really not
             * much we can do at this point, so we'll just instruct the user to use a more specific target name.
             */
            QStringList targetNames;
            std::transform(
                matchingDescriptionFiles.begin(),
                matchingDescriptionFiles.end(),
                std::back_inserter(targetNames),
                [](const QJsonValue& descriptionFile) {
                    return QString("\"" + descriptionFile.toObject().find("targetName")->toString().toLower() + "\"");
                }
            );

            throw Exception("Failed to resolve target description file for target \""
                + targetSignatureHex + "\" - ambiguous signature.\nThe signature is mapped to numerous targets: "
                + targetNames.join(", ").toStdString() + ".\n\nPlease update the target name in your Bloom " +
                "configuration to one of the above."
            );

        } else {
            throw Exception("Failed to resolve target description file for target \""
                + targetSignatureHex + "\" - invalid AVR8 target description mapping."
            );
        }

    } else {
        throw Exception("Failed to resolve target description file for target \""
            + targetSignatureHex + "\" - unknown target signature.");
    }
}

void TargetDescriptionFile::init(const QDomDocument& xml) {
    Targets::TargetDescription::TargetDescriptionFile::init(xml);

    this->loadDebugPhysicalInterfaces();
    this->loadPadDescriptors();
    this->loadTargetVariants();
}

QJsonObject TargetDescriptionFile::getTargetDescriptionMapping() {
    auto mappingFile = QFile(
        QString::fromStdString(Paths::resourcesDirPath() + "/TargetDescriptionFiles/AVR/Mapping.json")
    );

    if (!mappingFile.exists()) {
        throw Exception("Failed to load AVR target description mapping - mapping file not found");
    }

    mappingFile.open(QIODevice::ReadOnly);
    return QJsonDocument::fromJson(mappingFile.readAll()).object();
}

TargetSignature TargetDescriptionFile::getTargetSignature() const {
    auto& propertyGroups = this->propertyGroupsMappedByName;
    auto signaturePropertyGroupIt = propertyGroups.find("signatures");

    if (signaturePropertyGroupIt == propertyGroups.end()) {
        throw TargetDescriptionParsingFailureException("Signature property group not found");
    }

    auto signaturePropertyGroup = signaturePropertyGroupIt->second;
    auto& signatureProperties = signaturePropertyGroup.propertiesMappedByName;
    std::optional<unsigned char> signatureByteZero;
    std::optional<unsigned char> signatureByteOne;
    std::optional<unsigned char> signatureByteTwo;

    if (signatureProperties.find("signature0") != signatureProperties.end()) {
        signatureByteZero = static_cast<unsigned char>(
            signatureProperties.find("signature0")->second.value.toShort(nullptr, 16)
        );
    }

    if (signatureProperties.find("signature1") != signatureProperties.end()) {
        signatureByteOne = static_cast<unsigned char>(
            signatureProperties.find("signature1")->second.value.toShort(nullptr, 16)
        );
    }

    if (signatureProperties.find("signature2") != signatureProperties.end()) {
        signatureByteTwo = static_cast<unsigned char>(
            signatureProperties.find("signature2")->second.value.toShort(nullptr, 16)
        );
    }

    if (signatureByteZero.has_value() && signatureByteOne.has_value() && signatureByteTwo.has_value()) {
        return TargetSignature(signatureByteZero.value(), signatureByteOne.value(), signatureByteTwo.value());
    }

    throw TargetDescriptionParsingFailureException("Failed to extract target signature from AVR8 target description.");
}

Family TargetDescriptionFile::getFamily() const {
    static auto familyNameToEnums = TargetDescriptionFile::getFamilyNameToEnumMapping();
    auto familyName = this->deviceElement.attributes().namedItem("family").nodeValue().toLower().toStdString();

    if (familyName.empty()) {
        throw Exception("Could not find target family name in target description file.");
    }

    if (!familyNameToEnums.contains(familyName)) {
        throw Exception("Unknown family name in target description file.");
    }

    return familyNameToEnums.at(familyName);
}

TargetParameters TargetDescriptionFile::getTargetParameters() const {
    TargetParameters targetParameters;

    auto& peripheralModules = this->getPeripheralModulesMappedByName();
    auto& propertyGroups = this->getPropertyGroupsMappedByName();

    auto flashMemorySegment = this->getFlashMemorySegment();
    if (flashMemorySegment.has_value()) {
        targetParameters.flashSize = flashMemorySegment->size;
        targetParameters.flashStartAddress = flashMemorySegment->startAddress;

        if (flashMemorySegment->pageSize.has_value()) {
            targetParameters.flashPageSize = flashMemorySegment->pageSize.value();
        }
    }

    auto ramMemorySegment = this->getRamMemorySegment();
    if (ramMemorySegment.has_value()) {
        targetParameters.ramSize = ramMemorySegment->size;
        targetParameters.ramStartAddress = ramMemorySegment->startAddress;
    }

    auto ioMemorySegment = this->getIoMemorySegment();
    if (ioMemorySegment.has_value()) {
        targetParameters.mappedIoSegmentSize = ioMemorySegment->size;
        targetParameters.mappedIoSegmentStartAddress = ioMemorySegment->startAddress;
    }

    auto registerMemorySegment = this->getRegisterMemorySegment();
    if (registerMemorySegment.has_value()) {
        targetParameters.gpRegisterSize = registerMemorySegment->size;
        targetParameters.gpRegisterStartAddress = registerMemorySegment->startAddress;
    }

    auto eepromMemorySegment = this->getEepromMemorySegment();
    if (eepromMemorySegment.has_value()) {
        targetParameters.eepromSize = eepromMemorySegment->size;
        targetParameters.eepromStartAddress = eepromMemorySegment->startAddress;

        if (eepromMemorySegment->pageSize.has_value()) {
            targetParameters.eepromPageSize = eepromMemorySegment->pageSize.value();
        }
    }

    auto firstBootSectionMemorySegment = this->getFirstBootSectionMemorySegment();
    if (firstBootSectionMemorySegment.has_value()) {
        targetParameters.bootSectionStartAddress = firstBootSectionMemorySegment->startAddress / 2;
        targetParameters.bootSectionSize = firstBootSectionMemorySegment->size;
    }

    std::uint32_t cpuRegistersOffset = 0;

    if (peripheralModules.contains("cpu")) {
        auto cpuPeripheralModule = peripheralModules.at("cpu");

        if (cpuPeripheralModule.instancesMappedByName.contains("cpu")) {
            auto cpuInstance = cpuPeripheralModule.instancesMappedByName.at("cpu");

            if (cpuInstance.registerGroupsMappedByName.contains("cpu")) {
                cpuRegistersOffset = cpuInstance.registerGroupsMappedByName.at("cpu").offset.value_or(0);
            }
        }
    }

    auto statusRegister = this->getStatusRegister();
    if (statusRegister.has_value()) {
        targetParameters.statusRegisterStartAddress = cpuRegistersOffset + statusRegister->offset;
        targetParameters.statusRegisterSize = statusRegister->size;
    }

    auto stackPointerRegister = this->getStackPointerRegister();
    if (stackPointerRegister.has_value()) {
        targetParameters.stackPointerRegisterLowAddress = cpuRegistersOffset + stackPointerRegister->offset;
        targetParameters.stackPointerRegisterSize = stackPointerRegister->size;

    } else {
        // Sometimes the SP register is split into two register nodes, one for low, the other for high
        auto stackPointerLowRegister = this->getStackPointerLowRegister();
        auto stackPointerHighRegister = this->getStackPointerHighRegister();

        if (stackPointerLowRegister.has_value()) {
            targetParameters.stackPointerRegisterLowAddress = cpuRegistersOffset
                + stackPointerLowRegister->offset;
            targetParameters.stackPointerRegisterSize = stackPointerLowRegister->size;
        }

        if (stackPointerHighRegister.has_value()) {
            targetParameters.stackPointerRegisterSize =
                targetParameters.stackPointerRegisterSize.has_value() ?
                targetParameters.stackPointerRegisterSize.value() + stackPointerHighRegister->size :
                stackPointerHighRegister->size;
        }
    }

    auto supportedPhysicalInterfaces = this->getSupportedDebugPhysicalInterfaces();

    if (supportedPhysicalInterfaces.contains(PhysicalInterface::DEBUG_WIRE)
        || supportedPhysicalInterfaces.contains(PhysicalInterface::JTAG)
        ) {
        this->loadDebugWireAndJtagTargetParameters(targetParameters);
    }

    if (supportedPhysicalInterfaces.contains(PhysicalInterface::PDI)) {
        this->loadPdiTargetParameters(targetParameters);
    }

    if (supportedPhysicalInterfaces.contains(PhysicalInterface::UPDI)) {
        this->loadUpdiTargetParameters(targetParameters);
    }

    return targetParameters;
}

void TargetDescriptionFile::loadDebugPhysicalInterfaces() {
    auto interfaceNamesToInterfaces = std::map<std::string, PhysicalInterface>({
       {"updi", PhysicalInterface::UPDI},
       {"debugwire", PhysicalInterface::DEBUG_WIRE},
       {"jtag", PhysicalInterface::DEBUG_WIRE},
       {"pdi", PhysicalInterface::PDI},
    });

    for (const auto& [interfaceName, interface]: this->interfacesByName) {
        if (interfaceNamesToInterfaces.contains(interfaceName)) {
            this->supportedDebugPhysicalInterfaces.insert(interfaceNamesToInterfaces.at(interfaceName));
        }
    }
}

void TargetDescriptionFile::loadPadDescriptors() {
    auto& modules = this->getModulesMappedByName();
    auto portModule = (modules.contains("port")) ? std::optional(modules.find("port")->second) : std::nullopt;
    auto& peripheralModules = this->getPeripheralModulesMappedByName();

    if (peripheralModules.contains("port")) {
        auto portPeripheralModule = peripheralModules.find("port")->second;

        for (const auto& [instanceName, instance] : portPeripheralModule.instancesMappedByName) {
            if (instanceName.find("port") == 0) {
                auto portPeripheralRegisterGroup = (portPeripheralModule.registerGroupsMappedByName.contains(instanceName)) ?
                    std::optional(portPeripheralModule.registerGroupsMappedByName.find(instanceName)->second) :
                    std::nullopt;

                for (const auto& signal : instance.instanceSignals) {
                    if (!signal.index.has_value()) {
                        continue;
                    }

                    auto padDescriptor = PadDescriptor();
                    padDescriptor.name = signal.padName;
                    padDescriptor.gpioPinNumber = signal.index.value();

                    if (portModule.has_value() && portModule->registerGroupsMappedByName.contains(instanceName)) {
                        // We have register information for this port
                        auto registerGroup = portModule->registerGroupsMappedByName.find(instanceName)->second;

                        for (const auto& [registerName, portRegister] : registerGroup.registersMappedByName) {
                            if (registerName.find("port") == 0) {
                                // This is the data register for the port
                                padDescriptor.gpioPortSetAddress = portRegister.offset;
                                padDescriptor.gpioPortClearAddress = portRegister.offset;

                            } else if (registerName.find("pin") == 0) {
                                // This is the input data register for the port
                                padDescriptor.gpioPortInputAddress = portRegister.offset;

                            } else if (registerName.find("ddr") == 0) {
                                // This is the data direction register for the port
                                padDescriptor.ddrSetAddress = portRegister.offset;
                                padDescriptor.ddrClearAddress = portRegister.offset;
                            }
                        }

                    } else if (portModule.has_value() && portModule->registerGroupsMappedByName.contains("port")) {
                        // We have generic register information for all ports on the target
                        auto registerGroup = portModule->registerGroupsMappedByName.find("port")->second;

                        for (const auto& [registerName, portRegister] : registerGroup.registersMappedByName) {
                            if (registerName.find("outset") == 0) {
                                // Include the port register offset
                                padDescriptor.gpioPortSetAddress = (portPeripheralRegisterGroup.has_value()
                                    && portPeripheralRegisterGroup->offset.has_value()) ?
                                    portPeripheralRegisterGroup->offset.value_or(0) : 0;

                                padDescriptor.gpioPortSetAddress = padDescriptor.gpioPortSetAddress.value()
                                    + portRegister.offset;

                            } else if (registerName.find("outclr") == 0) {
                                padDescriptor.gpioPortClearAddress = (portPeripheralRegisterGroup.has_value()
                                    && portPeripheralRegisterGroup->offset.has_value()) ?
                                    portPeripheralRegisterGroup->offset.value_or(0) : 0;

                                padDescriptor.gpioPortClearAddress = padDescriptor.gpioPortClearAddress.value()
                                    + portRegister.offset;

                            } else if (registerName.find("dirset") == 0) {
                                padDescriptor.ddrSetAddress = (portPeripheralRegisterGroup.has_value()
                                    && portPeripheralRegisterGroup->offset.has_value()) ?
                                    portPeripheralRegisterGroup->offset.value_or(0) : 0;

                                padDescriptor.ddrSetAddress = padDescriptor.ddrSetAddress.value()
                                    + portRegister.offset;

                            } else if (registerName.find("dirclr") == 0) {
                                padDescriptor.ddrClearAddress = (portPeripheralRegisterGroup.has_value()
                                    && portPeripheralRegisterGroup->offset.has_value()) ?
                                    portPeripheralRegisterGroup->offset.value_or(0) : 0;

                                padDescriptor.ddrClearAddress = padDescriptor.ddrClearAddress.value()
                                    + portRegister.offset;

                            } else if (registerName == "in") {
                                padDescriptor.gpioPortInputAddress = (portPeripheralRegisterGroup.has_value()
                                    && portPeripheralRegisterGroup->offset.has_value()) ?
                                    portPeripheralRegisterGroup->offset.value_or(0) : 0;

                                padDescriptor.gpioPortInputAddress = padDescriptor.gpioPortInputAddress.value()
                                    + portRegister.offset;
                            }
                        }
                    }

                    this->padDescriptorsByName.insert(std::pair(padDescriptor.name, padDescriptor));
                }
            }
        }
    }
}

void TargetDescriptionFile::loadTargetVariants() {
    auto tdVariants = this->getVariants();
    auto tdPinoutsByName = this->getPinoutsMappedByName();
    auto& modules = this->getModulesMappedByName();

    for (const auto& tdVariant : tdVariants) {
        if (tdVariant.disabled) {
            continue;
        }

        auto targetVariant = TargetVariant();
        targetVariant.id = static_cast<int>(variants.size());
        targetVariant.name = tdVariant.name;
        targetVariant.packageName = tdVariant.package;

        if (tdVariant.package.find("QFP") == 0 || tdVariant.package.find("TQFP") == 0) {
            targetVariant.package = TargetPackage::QFP;

        } else if (tdVariant.package.find("PDIP") == 0 || tdVariant.package.find("DIP") == 0) {
            targetVariant.package = TargetPackage::DIP;

        } else if (tdVariant.package.find("QFN") == 0 || tdVariant.package.find("VQFN") == 0) {
            targetVariant.package = TargetPackage::QFN;

        } else if (tdVariant.package.find("SOIC") == 0) {
            targetVariant.package = TargetPackage::SOIC;

        } else if (tdVariant.package.find("SSOP") == 0) {
            targetVariant.package = TargetPackage::SSOP;
        }

        if (!tdPinoutsByName.contains(tdVariant.pinoutName)) {
            // Missing pinouts in the target description file
            continue;
        }

        auto tdPinout = tdPinoutsByName.find(tdVariant.pinoutName)->second;
        for (const auto& tdPin : tdPinout.pins) {
            auto targetPin = TargetPinDescriptor();
            targetPin.name = tdPin.pad;
            targetPin.padName = tdPin.pad;
            targetPin.number = tdPin.position;

            // TODO: REMOVE THIS:
            if (tdPin.pad.find("vcc") == 0
                || tdPin.pad.find("avcc") == 0
                || tdPin.pad.find("aref") == 0
                || tdPin.pad.find("avdd") == 0
                || tdPin.pad.find("vdd") == 0
                ) {
                targetPin.type = TargetPinType::VCC;

            } else if (tdPin.pad.find("gnd") == 0) {
                targetPin.type = TargetPinType::GND;
            }

            if (this->padDescriptorsByName.contains(targetPin.padName)) {
                auto& pad = this->padDescriptorsByName.at(targetPin.padName);
                if (pad.gpioPortSetAddress.has_value() && pad.ddrSetAddress.has_value()) {
                    targetPin.type = TargetPinType::GPIO;
                }
            }

            targetVariant.pinDescriptorsByNumber.insert(std::pair(targetPin.number, targetPin));
        }

        this->targetVariantsById.insert(std::pair(targetVariant.id, targetVariant));
    }
}


std::optional<MemorySegment> TargetDescriptionFile::getFlashMemorySegment() const {
    auto& addressMapping = this->addressSpacesMappedById;
    auto programAddressSpaceIt = addressMapping.find("prog");

    // Flash memory attributes are typically found in memory segments within the program address space.
    if (programAddressSpaceIt != addressMapping.end()) {
        auto& programAddressSpace = programAddressSpaceIt->second;
        auto& programMemorySegments = programAddressSpace.memorySegmentsByTypeAndName;

        if (programMemorySegments.find(MemorySegmentType::FLASH) != programMemorySegments.end()) {
            auto& flashMemorySegments = programMemorySegments.find(MemorySegmentType::FLASH)->second;

            /*
             * In AVR8 TDFs, flash memory segments are typically named "APP_SECTION", "PROGMEM" or "FLASH".
             */
            auto flashSegmentIt = flashMemorySegments.find("app_section") != flashMemorySegments.end() ?
                flashMemorySegments.find("app_section")
                : flashMemorySegments.find("progmem") != flashMemorySegments.end()
                ? flashMemorySegments.find("progmem") : flashMemorySegments.find("flash");

            if (flashSegmentIt != flashMemorySegments.end()) {
                return flashSegmentIt->second;
            }
        }
    }

    return std::nullopt;
}

std::optional<MemorySegment> TargetDescriptionFile::getRamMemorySegment() const {
    auto& addressMapping = this->addressSpacesMappedById;

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

std::optional<MemorySegment> TargetDescriptionFile::getIoMemorySegment() const {
    auto& addressMapping = this->addressSpacesMappedById;

    if (addressMapping.contains("data")) {
        auto& dataAddressSpace = addressMapping.at("data");
        auto& dataMemorySegments = dataAddressSpace.memorySegmentsByTypeAndName;

        if (dataMemorySegments.contains(MemorySegmentType::IO)) {
            auto& ramMemorySegments = dataMemorySegments.at(MemorySegmentType::IO);
            auto ramMemorySegmentIt = ramMemorySegments.begin();

            if (ramMemorySegmentIt != ramMemorySegments.end()) {
                return ramMemorySegmentIt->second;
            }
        }
    }

    return std::nullopt;
}

std::optional<MemorySegment> TargetDescriptionFile::getRegisterMemorySegment() const {
    auto& addressMapping = this->addressSpacesMappedById;

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
    auto& addressMapping = this->addressSpacesMappedById;

    if (addressMapping.contains("eeprom")) {
        auto& eepromAddressSpace = addressMapping.at("eeprom");
        auto& eepromAddressSpaceSegments = eepromAddressSpace.memorySegmentsByTypeAndName;

        if (eepromAddressSpaceSegments.contains(MemorySegmentType::EEPROM)) {
            return eepromAddressSpaceSegments.at(MemorySegmentType::EEPROM).begin()->second;
        }

    } else {
        // The EEPROM memory segment may be part of the data address space
        if (addressMapping.contains("data")) {
            auto dataAddressSpace = addressMapping.at("data");

            if (dataAddressSpace.memorySegmentsByTypeAndName.contains(MemorySegmentType::EEPROM)) {
                return dataAddressSpace.memorySegmentsByTypeAndName.at(MemorySegmentType::EEPROM).begin()->second;
            }
        }
    }

    return std::nullopt;
}

std::optional<MemorySegment> TargetDescriptionFile::getFirstBootSectionMemorySegment() const {
    auto& addressMapping = this->addressSpacesMappedById;
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

std::optional<MemorySegment> TargetDescriptionFile::getSignatureMemorySegment() const {
    if (this->addressSpacesMappedById.contains("signatures")) {
        auto& signaturesAddressSpace = this->addressSpacesMappedById.at("signatures");
        auto& signaturesAddressSpaceSegments = signaturesAddressSpace.memorySegmentsByTypeAndName;

        if (signaturesAddressSpaceSegments.contains(MemorySegmentType::SIGNATURES)) {
            return signaturesAddressSpaceSegments.at(MemorySegmentType::SIGNATURES).begin()->second;
        }

    } else {
        // The signatures memory segment may be part of the data address space
        if (this->addressSpacesMappedById.contains("data")) {
            auto dataAddressSpace = this->addressSpacesMappedById.at("data");

            if (dataAddressSpace.memorySegmentsByTypeAndName.contains(MemorySegmentType::SIGNATURES)) {
                auto& signatureSegmentsByName = dataAddressSpace.memorySegmentsByTypeAndName.at(
                    MemorySegmentType::SIGNATURES
                );

                if (signatureSegmentsByName.contains("signatures")) {
                    return signatureSegmentsByName.at("signatures");
                }
            }
        }
    }

    return std::nullopt;
}

std::optional<MemorySegment> TargetDescriptionFile::getFuseMemorySegment() const {
    if (this->addressSpacesMappedById.contains("data")) {
        auto dataAddressSpace = this->addressSpacesMappedById.at("data");

        if (dataAddressSpace.memorySegmentsByTypeAndName.contains(MemorySegmentType::FUSES)) {
            return dataAddressSpace.memorySegmentsByTypeAndName.at(
                MemorySegmentType::FUSES
            ).begin()->second;
        }
    }

    return std::nullopt;
}

std::optional<MemorySegment> TargetDescriptionFile::getLockbitsMemorySegment() const {
    if (this->addressSpacesMappedById.contains("data")) {
        auto dataAddressSpace = this->addressSpacesMappedById.at("data");

        if (dataAddressSpace.memorySegmentsByTypeAndName.contains(MemorySegmentType::LOCKBITS)) {
            return dataAddressSpace.memorySegmentsByTypeAndName.at(
                MemorySegmentType::LOCKBITS
            ).begin()->second;
        }
    }

    return std::nullopt;
}

std::optional<RegisterGroup> TargetDescriptionFile::getCpuRegisterGroup() const {
    auto& modulesByName = this->modulesMappedByName;

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
    auto& modulesByName = this->modulesMappedByName;

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
    auto& modulesByName = this->modulesMappedByName;

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
        auto& cpuRegisters = cpuRegisterGroup->registersMappedByName;

        if (cpuRegisters.contains("osccal")) {
            return cpuRegisters.at("osccal");

        } else if (cpuRegisters.contains("osccal0")) {
            return cpuRegisters.at("osccal0");

        } else if (cpuRegisters.contains("osccal1")) {
            return cpuRegisters.at("osccal1");

        } else if (cpuRegisters.contains("fosccal")) {
            return cpuRegisters.at("fosccal");

        } else if (cpuRegisters.contains("sosccala")) {
            return cpuRegisters.at("sosccala");
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

void TargetDescriptionFile::loadDebugWireAndJtagTargetParameters(TargetParameters& targetParameters) const {
    auto& peripheralModules = this->getPeripheralModulesMappedByName();
    auto& propertyGroups = this->getPropertyGroupsMappedByName();

    // OCD attributes can be found in property groups
    if (propertyGroups.contains("ocd")) {
        auto& ocdProperties = propertyGroups.at("ocd").propertiesMappedByName;

        if (ocdProperties.find("ocd_revision") != ocdProperties.end()) {
            targetParameters.ocdRevision = ocdProperties.find("ocd_revision")
                ->second.value.toUShort(nullptr, 10);
        }

        if (ocdProperties.find("ocd_datareg") != ocdProperties.end()) {
            targetParameters.ocdDataRegister = ocdProperties.find("ocd_datareg")
                ->second.value.toUShort(nullptr, 16);
        }
    }

    auto spmcsRegister = this->getSpmcsRegister();
    if (spmcsRegister.has_value()) {
        targetParameters.spmcRegisterStartAddress = spmcsRegister->offset;

    } else {
        auto spmcRegister = this->getSpmcRegister();
        if (spmcRegister.has_value()) {
            targetParameters.spmcRegisterStartAddress = spmcRegister->offset;
        }
    }

    auto osccalRegister = this->getOscillatorCalibrationRegister();
    if (osccalRegister.has_value()) {
        targetParameters.osccalAddress = osccalRegister->offset;
    }

    auto eepromAddressRegister = this->getEepromAddressRegister();
    if (eepromAddressRegister.has_value()) {
        targetParameters.eepromAddressRegisterLow = eepromAddressRegister->offset;
        targetParameters.eepromAddressRegisterHigh = (eepromAddressRegister->size == 2)
                                                            ? eepromAddressRegister->offset + 1 : eepromAddressRegister->offset;

    } else {
        auto eepromAddressLowRegister = this->getEepromAddressLowRegister();
        if (eepromAddressLowRegister.has_value()) {
            targetParameters.eepromAddressRegisterLow = eepromAddressLowRegister->offset;
            auto eepromAddressHighRegister = this->getEepromAddressHighRegister();

            if (eepromAddressHighRegister.has_value()) {
                targetParameters.eepromAddressRegisterHigh = eepromAddressHighRegister->offset;

            } else {
                targetParameters.eepromAddressRegisterHigh = eepromAddressLowRegister->offset;
            }
        }
    }

    auto eepromDataRegister = this->getEepromDataRegister();
    if (eepromDataRegister.has_value()) {
        targetParameters.eepromDataRegisterAddress = eepromDataRegister->offset;
    }

    auto eepromControlRegister = this->getEepromControlRegister();
    if (eepromControlRegister.has_value()) {
        targetParameters.eepromControlRegisterAddress = eepromControlRegister->offset;
    }
}

void TargetDescriptionFile::loadPdiTargetParameters(TargetParameters& targetParameters) const {
    auto& peripheralModules = this->getPeripheralModulesMappedByName();
    auto& propertyGroups = this->getPropertyGroupsMappedByName();

    if (propertyGroups.contains("pdi_interface")) {
        auto& pdiInterfaceProperties = propertyGroups.at("pdi_interface").propertiesMappedByName;

        if (pdiInterfaceProperties.contains("app_section_offset")) {
            targetParameters.appSectionPdiOffset = pdiInterfaceProperties
                .at("app_section_offset").value.toUInt(nullptr, 16);
        }

        if (pdiInterfaceProperties.contains("boot_section_offset")) {
            targetParameters.bootSectionPdiOffset = pdiInterfaceProperties
                .at("boot_section_offset").value.toUInt(nullptr, 16);
        }

        if (pdiInterfaceProperties.contains("datamem_offset")) {
            targetParameters.ramPdiOffset = pdiInterfaceProperties
                .at("datamem_offset").value.toUInt(nullptr, 16);
        }

        if (pdiInterfaceProperties.contains("eeprom_offset")) {
            targetParameters.eepromPdiOffset = pdiInterfaceProperties
                .at("eeprom_offset").value.toUInt(nullptr, 16);
        }

        if (pdiInterfaceProperties.contains("user_signatures_offset")) {
            targetParameters.userSignaturesPdiOffset = pdiInterfaceProperties
                .at("user_signatures_offset").value.toUInt(nullptr, 16);
        }

        if (pdiInterfaceProperties.contains("prod_signatures_offset")) {
            targetParameters.productSignaturesPdiOffset = pdiInterfaceProperties
                .at("prod_signatures_offset").value.toUInt(nullptr, 16);
        }

        if (pdiInterfaceProperties.contains("fuse_registers_offset")) {
            targetParameters.fuseRegistersPdiOffset = pdiInterfaceProperties
                .at("fuse_registers_offset").value.toUInt(nullptr, 16);
        }

        if (pdiInterfaceProperties.contains("lock_registers_offset")) {
            targetParameters.lockRegistersPdiOffset = pdiInterfaceProperties
                .at("lock_registers_offset").value.toUInt(nullptr, 16);
        }

        if (peripheralModules.contains("nvm")) {
            auto& nvmModule = peripheralModules.at("nvm");

            if (nvmModule.instancesMappedByName.contains("nvm")) {
                auto& nvmInstance = nvmModule.instancesMappedByName.at("nvm");

                if (nvmInstance.registerGroupsMappedByName.contains("nvm")) {
                    targetParameters.nvmBaseAddress = nvmInstance.registerGroupsMappedByName.at("nvm").offset;
                }
            }
        }
    }
}

void TargetDescriptionFile::loadUpdiTargetParameters(TargetParameters& targetParameters) const {
    auto& propertyGroups = this->getPropertyGroupsMappedByName();
    auto& peripheralModules = this->getPeripheralModulesMappedByName();
    auto modulesByName = this->getModulesMappedByName();

    if (peripheralModules.contains("nvmctrl")) {
        auto& nvmCtrlModule = peripheralModules.at("nvmctrl");

        if (nvmCtrlModule.instancesMappedByName.contains("nvmctrl")) {
            auto& nvmCtrlInstance = nvmCtrlModule.instancesMappedByName.at("nvmctrl");

            if (nvmCtrlInstance.registerGroupsMappedByName.contains("nvmctrl")) {
                targetParameters.nvmBaseAddress = nvmCtrlInstance.registerGroupsMappedByName.at("nvmctrl").offset;
            }
        }
    }

    if (propertyGroups.contains("updi_interface")) {
        auto& updiInterfaceProperties = propertyGroups.at("updi_interface").propertiesMappedByName;

        if (updiInterfaceProperties.contains("ocd_base_addr")) {
            targetParameters.ocdModuleAddress = updiInterfaceProperties
                .at("ocd_base_addr").value.toUShort(nullptr, 16);
        }

        if (updiInterfaceProperties.contains("progmem_offset")) {
            targetParameters.programMemoryUpdiStartAddress = updiInterfaceProperties
                .at("progmem_offset").value.toUInt(nullptr, 16);
        }
    }

    auto signatureMemorySegment = this->getSignatureMemorySegment();
    if (signatureMemorySegment.has_value()) {
        targetParameters.signatureSegmentStartAddress = signatureMemorySegment->startAddress;
        targetParameters.signatureSegmentSize = signatureMemorySegment->size;
    }

    auto fuseMemorySegment = this->getFuseMemorySegment();
    if (fuseMemorySegment.has_value()) {
        targetParameters.fuseSegmentStartAddress = fuseMemorySegment->startAddress;
        targetParameters.fuseSegmentSize = fuseMemorySegment->size;
    }

    auto lockbitsMemorySegment = this->getLockbitsMemorySegment();
    if (lockbitsMemorySegment.has_value()) {
        targetParameters.lockbitsSegmentStartAddress = lockbitsMemorySegment->startAddress;
    }
}
