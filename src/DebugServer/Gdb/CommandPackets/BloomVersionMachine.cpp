#include "BloomVersionMachine.hpp"

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>

#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"

#include "src/Application.hpp"

#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"

#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::ResponsePacket;

    using Exceptions::Exception;

    BloomVersionMachine::BloomVersionMachine(Monitor&& monitorPacket)
        : Monitor(std::move(monitorPacket))
    {}

    void BloomVersionMachine::handle(DebugSession& debugSession, TargetControllerService&) {
        Logger::debug("Handling BloomVersionMachine packet");

        debugSession.connection.writePacket(ResponsePacket(Services::StringService::toHex(
            QJsonDocument(QJsonObject({
                {"version", QString::fromStdString(Application::VERSION.toString())},
                {"components", QJsonObject({
                    {"major", Application::VERSION.getMajor()},
                    {"minor", Application::VERSION.getMinor()},
                    {"patch", Application::VERSION.getPatch()},
                })},
            })).toJson().toStdString()
        )));
    }
}
