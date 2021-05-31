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

TargetDescriptionFile::TargetDescriptionFile(
    const std::string& targetSignatureHex,
    std::optional<std::string> targetName
) {
    auto mapping = this->getTargetDescriptionMapping();
    auto qTargetSignatureHex = QString::fromStdString(targetSignatureHex);

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
            this->init(descriptionFilePath);

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
    auto propertyGroups = this->getPropertyGroupsMappedByName();
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
    static auto familyNameToEnums = this->getFamilyNameToEnumMapping();
    auto familyName = this->deviceElement.attributes().namedItem("family").nodeValue().toLower().toStdString();

    if (familyName.empty()) {
        throw Exception("Could not find target family name in target description file.");
    }

    if (familyNameToEnums.find(familyName) == familyNameToEnums.end()) {
        throw Exception("Unknown family name in target description file.");
    }

    return familyNameToEnums.find(familyName)->second;
}
