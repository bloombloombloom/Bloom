#include "HelpMonitorInfo.hpp"

#include <QFile>
#include <QString>

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"

#include "src/Services/PathService.hpp"
#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"

#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::ErrorResponsePacket;
    using ResponsePackets::ResponsePacket;

    using ::Exceptions::Exception;

    HelpMonitorInfo::HelpMonitorInfo(Monitor&& monitorPacket)
        : Monitor(std::move(monitorPacket))
    {}

    void HelpMonitorInfo::handle(DebugSession& debugSession, TargetControllerService&) {
        Logger::info("Handling HelpMonitorInfo packet");

        try {
            /*
             * The file GdbHelpMonitorInfo.txt is included in the binary image as a resource.
             * See src/DebugServer/CMakeLists.txt for more.
             */
            auto helpFile = QFile(
                QString::fromStdString(":/compiled/src/DebugServer/Gdb/Resources/GdbHelpMonitorInfo.txt")
            );

            if (!helpFile.open(QIODevice::ReadOnly)) {
                throw Exception(
                    "Failed to open GDB monitor info help file - please report this issue at "
                        + Services::PathService::homeDomainName() + "/report-issue"
                );
            }

            debugSession.connection.writePacket(
                ResponsePacket(Services::StringService::toHex(
                    "\n" + QTextStream(&helpFile).readAll().toUtf8().toStdString() + "\n"
                ))
            );

        } catch (const Exception& exception) {
            Logger::error(exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket());
        }
    }
}
