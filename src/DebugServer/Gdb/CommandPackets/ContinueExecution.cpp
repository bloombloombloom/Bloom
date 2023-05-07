#include "ContinueExecution.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::ErrorResponsePacket;
    using Exceptions::Exception;

    ContinueExecution::ContinueExecution(const RawPacket& rawPacket)
        : CommandPacket(rawPacket)
    {
        if (this->data.size() > 2) {
            this->fromAddress = static_cast<Targets::TargetProgramCounter>(
                std::stoi(std::string(this->data.begin() + 2, this->data.end()), nullptr, 16)
            );
        }
    }

    void ContinueExecution::handle(DebugSession& debugSession, TargetControllerService& targetControllerService) {
        Logger::info("Handling ContinueExecution packet");

        try {
            targetControllerService.continueTargetExecution(this->fromAddress, std::nullopt);
            debugSession.waitingForBreak = true;

        } catch (const Exception& exception) {
            Logger::error("Failed to continue execution on target - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket());
        }
    }
}
