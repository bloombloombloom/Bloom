#include "BloomVersion.hpp"

#include <string>

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"

#include "src/Application.hpp"
#include "src/Helpers/Paths.hpp"
#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    using TargetController::TargetControllerConsole;

    using ResponsePackets::ErrorResponsePacket;
    using ResponsePackets::ResponsePacket;

    using Exceptions::Exception;

    BloomVersion::BloomVersion(Monitor&& monitorPacket)
        : Monitor(std::move(monitorPacket))
    {}

    void BloomVersion::handle(DebugSession& debugSession, TargetControllerConsole&) {
        Logger::debug("Handling BloomVersion packet");

        debugSession.connection.writePacket(ResponsePacket(Packet::toHex(
            std::string(
                "Bloom v" + Application::VERSION.toString() + "\n"
                    + Paths::homeDomainName() + "\n"
                    + "Nav Mohammed\n"
            )
        )));
    }
}
