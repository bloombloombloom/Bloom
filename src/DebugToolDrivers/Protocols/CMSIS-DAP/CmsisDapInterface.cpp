#include "CmsisDapInterface.hpp"

#include <thread>

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/Command.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/Response.hpp"
#include "src/TargetController/Exceptions/DeviceCommunicationFailure.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap
{
    using namespace Bloom::Exceptions;

    void CmsisDapInterface::sendCommand(const Command& cmsisDapCommand) {
        if (this->msSendCommandDelay.count() > 0) {
            using namespace std::chrono;
            std::int64_t now = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
            std::int64_t difference = (now - this->lastCommandSentTimeStamp);

            if (difference < this->msSendCommandDelay.count()) {
                std::this_thread::sleep_for(milliseconds(this->msSendCommandDelay.count() - difference));
            }

            this->lastCommandSentTimeStamp = now;
        }

        this->getUsbHidInterface().write(static_cast<std::vector<unsigned char>>(cmsisDapCommand));
    }

    std::unique_ptr<Response> CmsisDapInterface::getResponse() {
        auto rawResponse = this->getUsbHidInterface().read(10000);

        if (rawResponse.empty()) {
            throw DeviceCommunicationFailure("Empty CMSIS-DAP response received");
        }

        auto response = std::make_unique<Response>(Response());
        response->init(rawResponse);
        return response;
    }

    std::unique_ptr<Response> CmsisDapInterface::sendCommandAndWaitForResponse(const Command& cmsisDapCommand) {
        this->sendCommand(cmsisDapCommand);
        auto response = this->getResponse();

        if (response->getResponseId() != cmsisDapCommand.getCommandId()) {
            // This response is not what we were expecting
            throw DeviceCommunicationFailure("Unexpected response to CMSIS-DAP command.");
        }

        return response;
    }
}
