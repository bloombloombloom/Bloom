#include <memory>
#include <chrono>
#include <thread>

#include "CmsisDapInterface.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/Command.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/Response.hpp"
#include "src/Exceptions/Exception.hpp"

using namespace Bloom::DebugToolDrivers::Protocols::CmsisDap;
using namespace Bloom::Exceptions;

void CmsisDapInterface::sendCommand(const Command& cmsisDapCommand) {
    if (this->msSendCommandDelay > 0) {
        using namespace std::chrono;
        long now = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
        long difference = (now - this->lastCommandSentTimeStamp);

        if (difference < this->msSendCommandDelay) {
            std::this_thread::sleep_for(milliseconds(this->msSendCommandDelay - difference));
        }

        this->lastCommandSentTimeStamp = now;
    }

    this->getUsbHidInterface().write(static_cast<std::vector<unsigned char>>(cmsisDapCommand));
}

std::unique_ptr<Response> CmsisDapInterface::getResponse() {
    auto rawResponse = this->getUsbHidInterface().read(5000);

    if (rawResponse.size() == 0) {
        throw Exception("Empty CMSIS-DAP response received");
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
        throw Exception("Unexpected response to CMSIS-DAP command.");
    }

    return response;
}
