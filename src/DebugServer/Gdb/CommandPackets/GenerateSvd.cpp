#include "GenerateSvd.hpp"

#include <QString>
#include <QFile>

#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Application.hpp"

#include "src/Helpers/Paths.hpp"
#include "src/Helpers/String.hpp"
#include "src/Logger/Logger.hpp"

#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    using TargetController::TargetControllerConsole;

    using ResponsePackets::ResponsePacket;
    using ResponsePackets::ErrorResponsePacket;
    using Exceptions::Exception;

    GenerateSvd::GenerateSvd(Monitor&& monitorPacket)
        : Monitor(std::move(monitorPacket))
        , sendOutput(this->commandOptions.contains("out"))
    {}

    void GenerateSvd::handle(DebugSession& debugSession, TargetControllerConsole&) {
        Logger::debug("Handling GenerateSvd packet");

        try {
            Logger::info("Generating SVD XML for current target");

            const auto& targetDescriptor = debugSession.gdbTargetDescriptor.targetDescriptor;

            const auto svdXml = this->generateSvd(
                targetDescriptor,
                debugSession.gdbTargetDescriptor.getMemoryOffset(Targets::TargetMemoryType::RAM)
            );

            if (this->sendOutput) {
                debugSession.connection.writePacket(ResponsePacket(String::toHex(svdXml.toString().toStdString())));
                return;
            }

            const auto svdOutputFilePath = Paths::projectDirPath() + "/" + targetDescriptor.name + ".svd";
            auto outputFile = QFile(QString::fromStdString(svdOutputFilePath));

            if (!outputFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
                throw Exception(
                    "Failed to open/create SVD output file (" + svdOutputFilePath + "). Check file permissions."
                );
            }

            outputFile.write(svdXml.toByteArray());
            outputFile.close();

            debugSession.connection.writePacket(ResponsePacket(String::toHex(
                "SVD output saved to " + svdOutputFilePath + "\n"
            )));

            Logger::info("SVD output saved to " + svdOutputFilePath);

        } catch (const Exception& exception) {
            Logger::error(exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket());
        }
    }

    QDomDocument GenerateSvd::generateSvd(
        const Targets::TargetDescriptor& targetDescriptor,
        std::uint32_t baseAddressOffset
    ) {
        auto document = QDomDocument();

        const auto createElement = [&document] (const QString& tagName, const QString& value) {
            auto element = document.createElement(tagName);
            auto textNode = document.createTextNode(value);
            element.appendChild(textNode);

            return element;
        };

        document.appendChild(document.createComment(
            " This SVD was generated by Bloom (https://bloom.oscillate.io/). "
            "Please report any issues via https://bloom.oscillate.io/report-issue "
        ));

        if (baseAddressOffset != 0) {
            document.appendChild(document.createComment(
                " Base addresses in this SVD have been offset by 0x" + QString::number(baseAddressOffset, 16)
                    + ". This offset is required for access via avr-gdb. "
            ));
        }

        auto deviceElement = document.createElement("device");

        deviceElement.setAttribute("schemaVersion", "1.3");
        deviceElement.setAttribute("xmlns:xs", "http://www.w3.org/2001/XMLSchema-instance");
        deviceElement.setAttribute(
            "xs:noNamespaceSchemaLocation",
            QString::fromStdString(Paths::homeDomainName() + "/assets/svd-schema.xsd")
        );

        deviceElement.appendChild(createElement("vendor", QString::fromStdString(targetDescriptor.vendorName)));
        deviceElement.appendChild(createElement("name", QString::fromStdString(targetDescriptor.name)));

        deviceElement.appendChild(document.createComment(
            " The version number below is that of the Bloom binary which generated this SVD. "
        ));
        deviceElement.appendChild(createElement("version", QString::fromStdString(Application::VERSION.toString())));

        deviceElement.appendChild(
            createElement(
                "description",
                QString::fromStdString(targetDescriptor.name) + " from "
                    + QString::fromStdString(targetDescriptor.vendorName)
            )
        );

        /*
         * TODO: These values should be part of the TargetDescriptor, but given that Bloom only supports 8-bit AVRs,
         *       it really doesn't matter ATM. Will fix it later (lol no I won't).
         */
        deviceElement.appendChild(createElement("addressUnitBits", "8"));
        deviceElement.appendChild(createElement("width", "8"));
        deviceElement.appendChild(createElement("size", "8"));

        deviceElement.appendChild(createElement("access", "read-only"));

        struct Peripheral {
            QString name;
            std::uint32_t baseAddress;

            Targets::TargetRegisterDescriptors registerDescriptors;
        };

        auto peripheralsByName = std::map<std::string, Peripheral>();

        for (const auto& [registerType, registerDescriptors] : targetDescriptor.registerDescriptorsByType) {
            if (registerDescriptors.empty()) {
                continue;
            }

            for (const auto& registerDescriptor : registerDescriptors) {
                if (
                    !registerDescriptor.startAddress.has_value()
                    || !registerDescriptor.name.has_value()
                    || registerDescriptor.name->empty()
                    || !registerDescriptor.groupName.has_value()
                    || (
                        registerDescriptor.type != Targets::TargetRegisterType::OTHER
                        && registerDescriptor.type != Targets::TargetRegisterType::PORT_REGISTER
                    )
                ) {
                    continue;
                }

                auto peripheralIt = peripheralsByName.find(*registerDescriptor.groupName);

                if (peripheralIt == peripheralsByName.end()) {
                    auto peripheral = Peripheral{
                        .name = QString::fromStdString(
                            *registerDescriptor.groupName
                        ).replace(QChar(' '), QChar('_')).toUpper(),
                        .baseAddress = baseAddressOffset
                    };

                    peripheralIt = peripheralsByName.insert(std::pair(*registerDescriptor.groupName, peripheral)).first;
                }

                peripheralIt->second.registerDescriptors.insert(registerDescriptor);
            }
        }

        auto peripheralsElement = document.createElement("peripherals");

        for (const auto& [peripheralName, peripheral] : peripheralsByName) {
            auto peripheralElement = document.createElement("peripheral");

            peripheralElement.appendChild(createElement("name", peripheral.name));
            peripheralElement.appendChild(createElement("baseAddress", "0x" + QString::number(peripheral.baseAddress, 16)));

            auto registersElement = document.createElement("registers");

            for (const auto& registerDescriptor : peripheral.registerDescriptors) {
                auto registerElement = document.createElement("register");

                registerElement.appendChild(
                    createElement(
                        "name",
                        QString::fromStdString(*registerDescriptor.name).replace(QChar(' '), QChar('_')).toUpper()
                    )
                );

                if (registerDescriptor.description.has_value()) {
                    registerElement.appendChild(
                        createElement("description", QString::fromStdString(*registerDescriptor.description))
                    );
                }

                registerElement.appendChild(
                    createElement("addressOffset", "0x" + QString::number(*registerDescriptor.startAddress, 16))
                );

                registerElement.appendChild(
                    createElement("size", QString::number(registerDescriptor.size * 8))
                );

                registerElement.appendChild(
                    createElement("access", registerDescriptor.writable ? "read-write" : "read-only")
                );

                registersElement.appendChild(registerElement);
            }

            peripheralElement.appendChild(registersElement);
            peripheralsElement.appendChild(peripheralElement);
        }

        deviceElement.appendChild(peripheralsElement);
        document.appendChild(deviceElement);
        return document;
    }
}
