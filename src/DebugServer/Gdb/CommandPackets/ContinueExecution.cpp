#include "ContinueExecution.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"

#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::ErrorResponsePacket;
    using ::Exceptions::Exception;

    ContinueExecution::ContinueExecution(const RawPacket& rawPacket)
        : CommandPacket(rawPacket)
    {
        if (this->data.size() > 2) {
            this->fromAddress = static_cast<Targets::TargetMemoryAddress>(
                Services::StringService::toUint32(std::string{this->data.begin() + 2, this->data.end()}, 16)
            );
        }
    }

    void ContinueExecution::handle(
        DebugSession& debugSession,
        const TargetDescriptor&,
        const Targets::TargetDescriptor&,
        TargetControllerService& targetControllerService
    ) {
        Logger::info("Handling ContinueExecution packet");

        try {
            {
                const auto atomicSession = targetControllerService.makeAtomicSession();
                if (this->fromAddress.has_value()) {
                    targetControllerService.setProgramCounter(*(this->fromAddress));
                }

                targetControllerService.resumeTargetExecution();
            }

            debugSession.waitingForBreak = true;

        } catch (const Exception& exception) {
            Logger::error("Failed to continue execution on target - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket{});
        }
    }
}
