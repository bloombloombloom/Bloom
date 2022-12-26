#include "StepExecution.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::ErrorResponsePacket;

    using Exceptions::Exception;

    StepExecution::StepExecution(const RawPacket& rawPacket)
        : CommandPacket(rawPacket)
    {
        if (this->data.size() > 1) {
            this->fromProgramCounter = static_cast<Targets::TargetProgramCounter>(
                std::stoi(std::string(this->data.begin(), this->data.end()), nullptr, 16)
            );
        }
    }

    void StepExecution::handle(DebugSession& debugSession, TargetControllerService& targetControllerService) {
        Logger::debug("Handling StepExecution packet");

        try {
            targetControllerService.stepTargetExecution(this->fromProgramCounter);
            debugSession.waitingForBreak = true;

        } catch (const Exception& exception) {
            Logger::error("Failed to step execution on target - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket());
        }
    }
}
