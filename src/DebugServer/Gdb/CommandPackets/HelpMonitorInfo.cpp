#include "HelpMonitorInfo.hpp"

#include <QFile>
#include <QString>

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"

#include "src/Helpers/Paths.hpp"
#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    using TargetController::TargetControllerConsole;

    using ResponsePackets::ErrorResponsePacket;
    using ResponsePackets::ResponsePacket;

    using Exceptions::Exception;

    HelpMonitorInfo::HelpMonitorInfo(Monitor&& monitorPacket)
        : Monitor(std::move(monitorPacket))
    {}

    void HelpMonitorInfo::handle(DebugSession& debugSession, TargetControllerConsole&) {
        Logger::debug("Handling HelpMonitorInfo packet");

        try {
            /*
             * The file gdbHelpMonitorInfo.txt is included in the binary image as a resource. See the root-level
             * CMakeLists.txt for more.
             */
            auto helpFile = QFile(
                QString::fromStdString(Paths::compiledResourcesPath() + "/resources/gdbHelpMonitorInfo.txt")
            );

            if (!helpFile.open(QIODevice::ReadOnly)) {
                throw Exception(
                    "Failed to open GDB monitor info help file - please report this issue at " + Paths::homeDomainName()
                        + "/report-issue"
                );
            }

            debugSession.connection.writePacket(
                ResponsePacket(Packet::toHex("\n" + QTextStream(&helpFile).readAll().toUtf8().toStdString() + "\n"))
            );

        } catch (const Exception& exception) {
            Logger::error(exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket());
        }
    }
}
