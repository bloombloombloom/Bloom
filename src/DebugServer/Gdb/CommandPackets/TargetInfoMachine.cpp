#include "TargetInfoMachine.hpp"

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"

#include "src/Application.hpp"
#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    using TargetController::TargetControllerConsole;

    using ResponsePackets::ResponsePacket;

    using Exceptions::Exception;

    TargetInfoMachine::TargetInfoMachine(Monitor&& monitorPacket)
        : Monitor(std::move(monitorPacket))
    {}

    void TargetInfoMachine::handle(DebugSession& debugSession, TargetControllerConsole&) {
        Logger::debug("Handling TargetInfoMachine packet");

        debugSession.connection.writePacket(ResponsePacket(Packet::toHex(
            QJsonDocument(
                this->generateTargetInfo(debugSession.gdbTargetDescriptor.targetDescriptor)
            ).toJson().toStdString()
        )));
    }

    QJsonObject TargetInfoMachine::generateTargetInfo(const Targets::TargetDescriptor& targetDescriptor) const {
        using Targets::TargetMemoryType;

        static const auto memoryTypeNamesByType = std::map<TargetMemoryType, QString>({
            {TargetMemoryType::FLASH, QString("Flash")},
            {TargetMemoryType::RAM, QString("RAM")},
            {TargetMemoryType::EEPROM, QString("EEPROM")},
        });

        auto memoryDescriptors = QJsonArray();

        for (const auto& [memoryType, memoryDescriptor] : targetDescriptor.memoryDescriptorsByType) {
            if (!memoryTypeNamesByType.contains(memoryType)) {
                continue;
            }

            memoryDescriptors.push_back(QJsonObject({
                {"name", memoryTypeNamesByType.at(memoryType)},
                {"size", static_cast<qint64>(memoryDescriptor.size())},
                {"addressRange", QJsonObject({
                    {"startAddress", static_cast<qint64>(memoryDescriptor.addressRange.startAddress)},
                    {"endAddress", static_cast<qint64>(memoryDescriptor.addressRange.endAddress)},
                })}
            }));
        }

        return QJsonObject({
            {"target", QJsonObject({
                {"name", QString::fromStdString(targetDescriptor.name)},
                {"id", QString::fromStdString(targetDescriptor.id)},
                {"memoryDescriptors", memoryDescriptors},
            })},
        });
    }
}