#pragma once

#include <memory>
#include <chrono>
#include <cstdint>

#include "src/DebugToolDrivers/Usb/Hid/HidInterface.hpp"

#include "Response.hpp"
#include "Command.hpp"

#include "src/TargetController/Exceptions/DeviceCommunicationFailure.hpp"

namespace DebugToolDrivers::Protocols::CmsisDap
{
    /**
     * The CmsisDapInterface class implements the CMSIS-DAP protocol.
     *
     * See https://www.keil.com/support/man/docs/dapdebug/dapdebug_introduction.htm for more on the CMSIS-DAP protocol.
     */
    class CmsisDapInterface
    {
    public:
        static constexpr auto CMSIS_COMMAND_DELAY_MAX = std::chrono::milliseconds{200};

        explicit CmsisDapInterface(Usb::HidInterface&& usbHidInterface);

        virtual ~CmsisDapInterface() = default;

        CmsisDapInterface(const CmsisDapInterface& other) = delete;
        CmsisDapInterface(CmsisDapInterface&& other) = delete;
        CmsisDapInterface& operator = (const CmsisDapInterface& other) = delete;
        CmsisDapInterface& operator = (CmsisDapInterface&& other) = delete;

        Usb::HidInterface& getUsbHidInterface() {
            return this->usbHidInterface;
        }

        std::size_t getUsbHidInputReportSize() {
            return this->usbHidInterface.inputReportSize;
        }

        void setCommandDelay(std::chrono::milliseconds commandDelay) {
            this->commandDelay = commandDelay;
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

            const auto rawResponse = this->getUsbHidInterface().read(std::chrono::milliseconds{60000});
            if (rawResponse.empty()) {
                throw Exceptions::DeviceCommunicationFailure{"Empty CMSIS-DAP response received"};
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

            if (response.id != cmsisDapCommand.id) {
                throw Exceptions::DeviceCommunicationFailure{"Unexpected response to CMSIS-DAP command."};
            }

            return response;
        }

    private:
        /**
         * All CMSIS-DAP devices employ the USB HID interface for communication.
         */
        Usb::HidInterface usbHidInterface;

        /**
         * Some CMSIS-DAP debug tools fail to operate properly when we send commands too quickly. Even if we've
         * received a response from every previous command.
         *
         * Because of this, we may need to enforce a minimum interval between sending CMSIS commands.
         */
        std::chrono::milliseconds commandDelay = std::chrono::milliseconds{0};
        std::int64_t lastCommandSentTimeStamp = 0;
    };
}
