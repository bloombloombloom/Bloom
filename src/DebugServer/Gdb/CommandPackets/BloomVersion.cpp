#include "BloomVersion.hpp"

#include <string>

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"

#include "src/Application.hpp"

#include "src/Services/PathService.hpp"
#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::ErrorResponsePacket;
    using ResponsePackets::ResponsePacket;

    BloomVersion::BloomVersion(Monitor&& monitorPacket)
        : Monitor(std::move(monitorPacket))
    {}

    void BloomVersion::handle(DebugSession& debugSession, TargetControllerService&) {
        Logger::info("Handling BloomVersion packet");

        debugSession.connection.writePacket(ResponsePacket(Services::StringService::toHex(
            std::string(
                "Bloom v" + Application::VERSION.toString() + "\n"
                    + Services::PathService::homeDomainName() + "\n"
                    + "Nav Mohammed\n"
            )
        )));
    }
}
