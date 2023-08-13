#include "EdbgInterface.hpp"

#include <memory>

#include "src/TargetController/Exceptions/DeviceCommunicationFailure.hpp"

namespace DebugToolDrivers::Protocols::CmsisDap::Edbg
{
    using namespace Exceptions;

    EdbgInterface::EdbgInterface(Usb::HidInterface&& cmsisHidInterface)
        : CmsisDapInterface(std::move(cmsisHidInterface))
    {}

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

        if (avrEventResponse.id != 0x82) {
            throw DeviceCommunicationFailure("Unexpected response to AvrEventCommand from device");
        }

        return !avrEventResponse.eventData.empty() ? std::optional(avrEventResponse) : std::nullopt;
    }

    std::vector<Protocols::CmsisDap::Edbg::Avr::AvrResponse> EdbgInterface::requestAvrResponses() {
        using Protocols::CmsisDap::Edbg::Avr::AvrResponseCommand;

        std::vector<Protocols::CmsisDap::Edbg::Avr::AvrResponse> responses;
        AvrResponseCommand responseCommand;

        auto avrResponse = this->sendCommandAndWaitForResponse(responseCommand);
        responses.push_back(avrResponse);
        const auto fragmentCount = avrResponse.fragmentCount;

        while (responses.size() < fragmentCount) {
            // There are more response packets
            auto avrResponse = this->sendCommandAndWaitForResponse(responseCommand);

            if (avrResponse.fragmentCount != fragmentCount) {
                throw DeviceCommunicationFailure(
                    "Failed to fetch AvrResponse objects - invalid fragment count returned."
                );
            }

            if (avrResponse.fragmentCount == 0 && avrResponse.fragmentNumber == 0) {
                throw DeviceCommunicationFailure(
                    "Failed to fetch AvrResponse objects - unexpected empty response"
                );
            }

            if (avrResponse.fragmentNumber == 0) {
                // End of response data ( &this packet can be ignored)
                break;
            }

            responses.push_back(avrResponse);
        }

        return responses;
    }
}
