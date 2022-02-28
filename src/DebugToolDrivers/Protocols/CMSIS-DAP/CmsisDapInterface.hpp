#pragma once

#include <memory>
#include <chrono>
#include <cstdint>

#include "src/DebugToolDrivers/USB/HID/HidInterface.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/Response.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/Command.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/AvrCommand.hpp"

#include "src/TargetController/Exceptions/DeviceCommunicationFailure.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap
{
    /**
     * The CmsisDapInterface class implements the CMSIS-DAP protocol.
     *
     * See https://www.keil.com/support/man/docs/dapdebug/dapdebug_introduction.htm for more on the CMSIS-DAP protocol.
     */
    class CmsisDapInterface
    {
    public:
        explicit CmsisDapInterface() = default;
        virtual ~CmsisDapInterface() = default;

        CmsisDapInterface(const CmsisDapInterface& other) = default;
        CmsisDapInterface(CmsisDapInterface&& other) = default;

        CmsisDapInterface& operator = (const CmsisDapInterface& other) = default;
        CmsisDapInterface& operator = (CmsisDapInterface&& other) = default;

        Usb::HidInterface& getUsbHidInterface() {
            return this->usbHidInterface;
        }

        std::size_t getUsbHidInputReportSize() {
            return this->usbHidInterface.getInputReportSize();
        }

        void setMinimumCommandTimeGap(std::chrono::milliseconds commandTimeGap) {
            this->msSendCommandDelay = commandTimeGap;
        }

        /**
         * Sends a CMSIS-DAP command to the device.
         *
         * @param cmsisDapCommand
         */
        virtual void sendCommand(const Command& cmsisDapCommand);

        /**
         * Listens for a CMSIS-DAP response from the device.
         *
         * @TODO: There is a hard-coded timeout in this method. Review.
         *
         * @return
         *  An instance to ResponseType, which must be derived from the Response class. The instance is constructed via
         *  its raw buffer constructor: ResponseType(const std::vector<unsigned char>&).
         */
        template<class ResponseType>
        auto getResponse() {
            static_assert(
                std::is_base_of<Response, ResponseType>::value,
                "CMSIS Response type must be derived from the Response class."
            );

            const auto rawResponse = this->getUsbHidInterface().read(10000);

            if (rawResponse.empty()) {
                throw Exceptions::DeviceCommunicationFailure("Empty CMSIS-DAP response received");
            }

            return ResponseType(rawResponse);
        }

        /**
         * Sends a CMSIS-DAP command and waits for a response.
         *
         * @param cmsisDapCommand
         *
         * @return
         *  An instance of the ExpectedResponseType alias defined in CommandType.
         *  See the CmsisDapInterface::getResponse() template function for more.
         */
        template<class CommandType>
        auto sendCommandAndWaitForResponse(const CommandType& cmsisDapCommand) {
            static_assert(
                std::is_base_of<Command, CommandType>::value,
                "CMSIS Command type must be derived from the Command class."
            );

            static_assert(
                std::is_base_of<Response, typename CommandType::ExpectedResponseType>::value,
                "CMSIS Command type must specify a valid expected response type, derived from the Response class."
            );

            this->sendCommand(cmsisDapCommand);
            auto response = this->getResponse<typename CommandType::ExpectedResponseType>();

            if (response.getResponseId() != cmsisDapCommand.getCommandId()) {
                throw Exceptions::DeviceCommunicationFailure("Unexpected response to CMSIS-DAP command.");
            }

            return response;
        }

    private:
        /**
         * All CMSIS-DAP devices employ the USB HID interface for communication.
         *
         * For many CMSIS-DAP devices, the USB HID interface parameters (interface number, endpoint config, etc) vary
         * amongst devices, so we'll need to be able to preActivationConfigure the CMSISDAPInterface from a
         * higher level. For an example, see the constructor of the AtmelIce device class.
         */
        Usb::HidInterface usbHidInterface = Usb::HidInterface();

        /**
         * Some CMSIS-DAP debug tools fail to operate properly when we send commands too quickly. Even if we've
         * received a response from every previous command.
         *
         * Because of this, we may need to enforce a minimum time gap between sending CMSIS commands.
         * Setting msSendCommandDelay to any value above 0 will enforce an x millisecond gap between each command
         * being sent, where x is the value of msSendCommandDelay.
         */
        std::chrono::milliseconds msSendCommandDelay = std::chrono::milliseconds(0);
        std::int64_t lastCommandSentTimeStamp = 0;
    };
}
