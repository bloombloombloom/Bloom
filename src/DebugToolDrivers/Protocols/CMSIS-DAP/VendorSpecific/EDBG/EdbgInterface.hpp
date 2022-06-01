#pragma once

#include <memory>

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/CmsisDapInterface.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/AvrCommand.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/AvrResponse.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/AvrEventCommand.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/AvrEvent.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AvrCommandFrame.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/AvrResponseCommand.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/ResponseFrames/AvrResponseFrame.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AvrCommandFrame.hpp"
#include "src/TargetController/Exceptions/DeviceCommunicationFailure.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg
{
    /**
     * The EdbgInterface class implements the EDBG sub-protocol, which takes the form of numerous CMSIS-DAP vendor
     * commands.
     */
    class EdbgInterface: public CmsisDapInterface
    {
    public:
        explicit EdbgInterface() = default;

        /**
         * Send an AvrCommandFrame to the debug tool and wait for a response.
         *
         * NOTE: The response this method waits for is *not* an AvrResponseFrame, but rather, just a response from
         * the debug tool indicating successful receipt of the AvrCommandFrame.
         * See EdbgInterface::sendAvrCommandFrameAndWaitForResponseFrame().
         *
         * @param avrCommandFrame
         * @return
         */
        template <class PayloadContainerType>
        Protocols::CmsisDap::Response sendAvrCommandFrameAndWaitForResponse(
            const Protocols::CmsisDap::Edbg::Avr::AvrCommandFrame<PayloadContainerType>& avrCommandFrame
        ) {
            // An AVR command frame can be split into multiple CMSIS-DAP commands. Each command
            // containing a fragment of the AvrCommandFrame.
            return this->sendAvrCommandsAndWaitForResponse(avrCommandFrame.generateAvrCommands(
                this->getUsbHidInputReportSize() - 4 // Minus 4 to accommodate AVR command bytes
            ));
        }

        virtual Protocols::CmsisDap::Response sendAvrCommandsAndWaitForResponse(
            const std::vector<Avr::AvrCommand>& avrCommands
        );

        template<class CommandFrameType>
        typename CommandFrameType::ExpectedResponseFrameType sendAvrCommandFrameAndWaitForResponseFrame(
            const CommandFrameType& avrCommandFrame
        ) {
            static_assert(
                std::is_base_of<
                    Protocols::CmsisDap::Edbg::Avr::AvrResponseFrame,
                    typename CommandFrameType::ExpectedResponseFrameType
                >::value,
                "AVR Command must specify a valid response frame type, derived from AvrResponseFrame."
            );

            auto response = this->sendAvrCommandFrameAndWaitForResponse(avrCommandFrame);

            if (response.getData()[0] != 0x01) {
                // The last response packet should always acknowledge receipt of the AvrCommandFrame
                throw Exceptions::DeviceCommunicationFailure(
                    "Failed to send AvrCommandFrame to device - device did not acknowledge receipt."
                );
            }

            auto responses = this->requestAvrResponses();
            auto responseFrame = typename CommandFrameType::ExpectedResponseFrameType();
            responseFrame.initFromAvrResponses(responses);
            return responseFrame;
        }

        virtual std::optional<Protocols::CmsisDap::Edbg::Avr::AvrEvent> requestAvrEvent();

    private:
        virtual std::vector<Protocols::CmsisDap::Edbg::Avr::AvrResponse> requestAvrResponses();
    };
}
