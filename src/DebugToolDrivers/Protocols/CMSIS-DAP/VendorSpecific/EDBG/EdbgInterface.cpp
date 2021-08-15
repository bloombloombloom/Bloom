#include "EdbgInterface.hpp"

#include <cstdint>
#include <cstring>
#include <memory>

#include "src/TargetController/Exceptions/DeviceCommunicationFailure.hpp"

using namespace Bloom::DebugToolDrivers;
using namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg;
using namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr;
using namespace Bloom::Exceptions;
using namespace Bloom::Exceptions;

Protocols::CmsisDap::Response EdbgInterface::sendAvrCommandFrameAndWaitForResponse(
    const Protocols::CmsisDap::Edbg::Avr::AvrCommandFrame& avrCommandFrame
) {
    // An AVR command frame can be split into multiple CMSIS-DAP commands. Each command
    // containing a fragment of the AvrCommandFrame.

    // Minus 3 to accommodate AVR command meta data
    std::size_t maximumCommandPacketSize = (this->getUsbHidInputReportSize() - 3);

    auto avrCommands = avrCommandFrame.generateAvrCommands(maximumCommandPacketSize);

    for (auto& avrCommand : avrCommands) {
        // Send command to device
        auto response = this->sendCommandAndWaitForResponse(avrCommand);

        if (&avrCommand ==& avrCommands.back()) {
            return* response;
        }
    }

    // This should never happen
    throw DeviceCommunicationFailure(
        "Cannot send AVR command frame - failed to generate CMSIS-DAP Vendor (AVR) commands"
    );
}

Protocols::CmsisDap::Edbg::Avr::AvrResponse EdbgInterface::getAvrResponse() {
    auto cmsisResponse = this->getResponse();

    if (cmsisResponse->getResponseId() == 0x81) {
        // This is an AVR_RSP response
        auto avrResponse = Protocols::CmsisDap::Edbg::Avr::AvrResponse();
        avrResponse.init(*cmsisResponse);
        return avrResponse;
    } else {
        throw DeviceCommunicationFailure("Unexpected response to AvrResponseCommand from device");
    }
}

std::optional<Protocols::CmsisDap::Edbg::Avr::AvrEvent> EdbgInterface::requestAvrEvent() {
    this->sendCommand(AvrEventCommand());
    auto cmsisResponse = this->getResponse();

    if (cmsisResponse->getResponseId() == 0x82) {
        // This is an AVR_EVT response
        auto avrEvent = Protocols::CmsisDap::Edbg::Avr::AvrEvent();
        avrEvent.init(*cmsisResponse);
        return avrEvent.getEventDataSize() > 0 ? std::optional(avrEvent): std::nullopt;
    } else {
        throw DeviceCommunicationFailure("Unexpected response to AvrEventCommand from device");
    }
}

std::vector<Protocols::CmsisDap::Edbg::Avr::AvrResponse> EdbgInterface::requestAvrResponses() {
    std::vector<Protocols::CmsisDap::Edbg::Avr::AvrResponse> responses;
    AvrResponseCommand responseCommand;

    this->sendCommand(responseCommand);
    auto response = this->getAvrResponse();
    responses.push_back(response);
    int fragmentCount = response.getFragmentCount();

    while (responses.size() < fragmentCount) {
        // There are more response packets
        this->sendCommand(responseCommand);
        response = this->getAvrResponse();

        if (response.getFragmentCount() != fragmentCount) {
            throw DeviceCommunicationFailure(
                "Failed to fetch AVRResponse objects - invalid fragment count returned."
            );
        }

        if (response.getFragmentCount() == 0 && response.getFragmentNumber() == 0) {
            throw DeviceCommunicationFailure("Failed to fetch AVRResponse objects - unexpected empty response");

        } else if (response.getFragmentNumber() == 0) {
            // End of response data ( &this packet can be ignored)
            break;
        }

        responses.push_back(response);
    }

    return responses;
}
