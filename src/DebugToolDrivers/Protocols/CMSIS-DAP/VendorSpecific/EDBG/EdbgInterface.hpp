#pragma once

#include <memory>

#include "src/Exceptions/Exception.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/CmsisDapInterface.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/AvrCommand.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/AvrEventCommand.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/AvrEvent.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AvrCommandFrame.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/AvrResponseCommand.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/ResponseFrames/AvrResponseFrame.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AvrCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg
{
    using namespace Protocols::CmsisDap::Edbg::Avr;
    using namespace Exceptions;

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
        virtual Protocols::CmsisDap::Response sendAvrCommandFrameAndWaitForResponse(
            const Protocols::CmsisDap::Edbg::Avr::AvrCommandFrame& avrCommandFrame
        );

        Protocols::CmsisDap::Edbg::Avr::AvrResponse getAvrResponse();

        virtual std::vector<Protocols::CmsisDap::Edbg::Avr::AvrResponse> requestAvrResponses();

        virtual std::optional<Protocols::CmsisDap::Edbg::Avr::AvrEvent> requestAvrEvent();


        template<class CommandFrameType>
        typename CommandFrameType::ResponseFrameType sendAvrCommandFrameAndWaitForResponseFrame(
            const CommandFrameType& avrCommandFrame
        ) {
            static_assert(
                std::is_base_of<AvrCommandFrame, CommandFrameType>::value,
                "AVR Command must be base of AvrCommandFrame."
            );

            static_assert(
                std::is_base_of<AvrResponseFrame, typename CommandFrameType::ResponseFrameType>::value,
                "AVR Command must specify a valid response frame type, derived from AvrResponseFrame."
            );

            auto response = this->sendAvrCommandFrameAndWaitForResponse(avrCommandFrame);

            if (response.getData()[0] != 0x01) {
                // The last response packet should always acknowledge receipt of the AvrCommandFrame
                throw Exception("Failed to send AvrCommandFrame to device - device did not acknowledge receipt.");
            }

            auto responses = this->requestAvrResponses();
            auto responseFrame = typename CommandFrameType::ResponseFrameType();
            responseFrame.initFromAvrResponses(responses);
            return responseFrame;
        }
    };
}
