#include "BloomVersionMachine.hpp"

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>

#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"

#include "src/Application.hpp"

#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::ResponsePacket;

    BloomVersionMachine::BloomVersionMachine(Monitor&& monitorPacket)
        : Monitor(std::move(monitorPacket))
    {}

    void BloomVersionMachine::handle(
        DebugSession& debugSession,
        const TargetDescriptor&,
        const Targets::TargetDescriptor&,
        TargetControllerService&
    ) {
        Logger::info("Handling BloomVersionMachine packet");

        debugSession.connection.writePacket(ResponsePacket{Services::StringService::toHex(
            QJsonDocument{
                QJsonObject{
                    {"version", QString::fromStdString(Application::VERSION.toString())},
                    {
                        "components",
                        QJsonObject{
                            {"major", Application::VERSION.major},
                            {"minor", Application::VERSION.minor},
                            {"patch", Application::VERSION.patch},
                        }
                    },
                }
            }.toJson().toStdString()
        )});
    }
}
