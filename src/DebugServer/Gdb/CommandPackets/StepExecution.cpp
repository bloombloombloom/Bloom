#include "StepExecution.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::ErrorResponsePacket;

    using ::Exceptions::Exception;

    StepExecution::StepExecution(const RawPacket& rawPacket)
        : CommandPacket(rawPacket)
    {
        if (this->data.size() > 2) {
            this->fromAddress = static_cast<Targets::TargetProgramCounter>(
                std::stoi(std::string(this->data.begin() + 2, this->data.end()), nullptr, 16)
            );
        }
    }

    void StepExecution::handle(DebugSession& debugSession, TargetControllerService& targetControllerService) {
        Logger::info("Handling StepExecution packet");

        try {
            targetControllerService.stepTargetExecution(this->fromAddress);
            debugSession.waitingForBreak = true;

        } catch (const Exception& exception) {
            Logger::error("Failed to step execution on target - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket());
        }
    }
}
