#pragma once

#include <memory>
#include <chrono>

#include "src/DebugToolDrivers/USB/HID/HidInterface.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/Response.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/Command.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/AvrCommand.hpp"

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

        size_t getUsbHidInputReportSize() {
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
        virtual void sendCommand(const Protocols::CmsisDap::Command& cmsisDapCommand);

        /**
         * Listens for a CMSIS-DAP response from the device.
         *
         * @TODO: There is a hard-coded timeout in this method. Review.
         *
         * @return
         *  The parsed response.
         */
        virtual std::unique_ptr<Protocols::CmsisDap::Response> getResponse();

        /**
         * Sends a CMSIS-DAP command and waits for a response.
         *
         * @param cmsisDapCommand
         *
         * @return
         *  The parsed response.
         */
        virtual std::unique_ptr<Protocols::CmsisDap::Response> sendCommandAndWaitForResponse(
            const Protocols::CmsisDap::Command& cmsisDapCommand
        );

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
