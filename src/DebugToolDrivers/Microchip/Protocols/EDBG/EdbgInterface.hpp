#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/CmsisDapInterface.hpp"

#include "AVR/AvrCommand.hpp"
#include "AVR/AvrResponse.hpp"
#include "AVR/AvrEventCommand.hpp"
#include "AVR/AvrEvent.hpp"
#include "AVR/CommandFrames/AvrCommandFrame.hpp"
#include "AVR/AvrResponseCommand.hpp"
#include "AVR/ResponseFrames/AvrResponseFrame.hpp"
#include "AVR/CommandFrames/AvrCommandFrame.hpp"

#include "src/TargetController/Exceptions/DeviceCommunicationFailure.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg
{
    /**
     * The EdbgInterface class implements the EDBG sub-protocol, which takes the form of numerous CMSIS-DAP vendor
     * commands.
     */
class EdbgInterface: public ::DebugToolDrivers::Protocols::CmsisDap::CmsisDapInterface
    {
    public:
        explicit EdbgInterface(Usb::HidInterface&& cmsisHidInterface);

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
        ::DebugToolDrivers::Protocols::CmsisDap::Response sendAvrCommandFrameAndWaitForResponse(
            const Avr::AvrCommandFrame<PayloadContainerType>& avrCommandFrame
        ) {
            /*
             * An AVR command frame can be split into multiple CMSIS-DAP commands. Each command containing a fragment
             * of the AvrCommandFrame.
             */
            return this->sendAvrCommandsAndWaitForResponse(avrCommandFrame.generateAvrCommands(
                this->getUsbHidInputReportSize() - 4 // Minus 4 to accommodate AVR command bytes
            ));
        }

        virtual ::DebugToolDrivers::Protocols::CmsisDap::Response sendAvrCommandsAndWaitForResponse(
            const std::vector<Avr::AvrCommand>& avrCommands
        );

        template<class CommandFrameType>
        typename CommandFrameType::ExpectedResponseFrameType sendAvrCommandFrameAndWaitForResponseFrame(
            const CommandFrameType& avrCommandFrame
        ) {
            static_assert(
                std::is_base_of<
                    Avr::AvrResponseFrame,
                    typename CommandFrameType::ExpectedResponseFrameType
                >::value,
                "AVR Command must specify a valid response frame type, derived from AvrResponseFrame."
            );

            const auto response = this->sendAvrCommandFrameAndWaitForResponse(avrCommandFrame);

            if (response.data[0] != 0x01) {
                // The last response packet should always acknowledge receipt of the AvrCommandFrame
                throw Exceptions::DeviceCommunicationFailure(
                    "Failed to send AvrCommandFrame to device - device did not acknowledge receipt."
                );
            }

            return typename CommandFrameType::ExpectedResponseFrameType(this->requestAvrResponses());
        }

        virtual std::optional<Avr::AvrEvent> requestAvrEvent();

    private:
        virtual std::vector<Avr::AvrResponse> requestAvrResponses();
    };
}
