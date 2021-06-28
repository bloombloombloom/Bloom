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
