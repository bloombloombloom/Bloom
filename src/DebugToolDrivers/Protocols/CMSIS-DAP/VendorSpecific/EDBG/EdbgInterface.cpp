#include "EdbgInterface.hpp"

#include <memory>

#include "src/TargetController/Exceptions/DeviceCommunicationFailure.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg
{
    using namespace Bloom::Exceptions;

    Protocols::CmsisDap::Response EdbgInterface::sendAvrCommandsAndWaitForResponse(
        const std::vector<Avr::AvrCommand>& avrCommands
    ) {
        for (const auto& avrCommand : avrCommands) {
            // Send command to device
            auto response = this->sendCommandAndWaitForResponse(avrCommand);

            if (&avrCommand == &avrCommands.back()) {
                return response;
            }
        }

        // This should never happen
        throw DeviceCommunicationFailure(
            "Cannot send AVR command frame - failed to generate CMSIS-DAP Vendor (AVR) commands"
        );
    }

    std::optional<Protocols::CmsisDap::Edbg::Avr::AvrEvent> EdbgInterface::requestAvrEvent() {
        auto avrEventResponse = this->sendCommandAndWaitForResponse(Avr::AvrEventCommand());

        if (avrEventResponse.getResponseId() != 0x82) {
            throw DeviceCommunicationFailure("Unexpected response to AvrEventCommand from device");
        }

        return avrEventResponse.getEventDataSize() > 0 ? std::optional(avrEventResponse) : std::nullopt;
    }

    std::vector<Protocols::CmsisDap::Edbg::Avr::AvrResponse> EdbgInterface::requestAvrResponses() {
        using Protocols::CmsisDap::Edbg::Avr::AvrResponseCommand;

        std::vector<Protocols::CmsisDap::Edbg::Avr::AvrResponse> responses;
        AvrResponseCommand responseCommand;

        auto avrResponse = this->sendCommandAndWaitForResponse(responseCommand);
        responses.push_back(avrResponse);
        const auto fragmentCount = avrResponse.getFragmentCount();

        while (responses.size() < fragmentCount) {
            // There are more response packets
            auto avrResponse = this->sendCommandAndWaitForResponse(responseCommand);

            if (avrResponse.getFragmentCount() != fragmentCount) {
                throw DeviceCommunicationFailure(
                    "Failed to fetch AVRResponse objects - invalid fragment count returned."
                );
            }

            if (avrResponse.getFragmentCount() == 0 && avrResponse.getFragmentNumber() == 0) {
                throw DeviceCommunicationFailure(
                    "Failed to fetch AVRResponse objects - unexpected empty response"
                );
            }

            if (avrResponse.getFragmentNumber() == 0) {
                // End of response data ( &this packet can be ignored)
                break;
            }

            responses.push_back(avrResponse);
        }

        return responses;
    }
}
