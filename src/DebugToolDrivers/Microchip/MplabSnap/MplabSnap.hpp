#pragma once

#include <cstdint>
#include <vector>
#include <memory>

#include "src/DebugToolDrivers/DebugTool.hpp"
#include "src/DebugToolDrivers/USB/UsbDevice.hpp"
#include "src/DebugToolDrivers/USB/HID/HidInterface.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/CmsisDapInterface.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/EdbgInterface.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/EdbgAvr8Interface.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AvrCommandFrames.hpp"

namespace Bloom::DebugToolDrivers
{
    /**
     * The MPLAB Snap device is a hybrid device - that is, it can present itself as an "MPLAB Snap ICD" device, as well
     * as an EDBG (Embedded Debugger) device. The device switches between these two modes via a firmware update, issued
     * by Microchip software (the MPLAB IDE and IPE). It appears that it can only interface with AVR targets using the
     * EDBG firmware, which, apparently, is why it is known to be in "AVR mode" when it presents itself as an EDBG
     * device.
     *
     * This debug tool driver currently only supports the device when in AVR mode. Because the device uses different
     * vendor & product IDs depending on the mode, it is easy to determine which is which. In fact, Bloom will not even
     * recognise the device if it's not in AVR mode.
     *
     * Communication:
     *  Uses the same EDBG protocol as described in the AtmelIce driver. See the AtmelIce debug tool class for more.
     *
     * USB Setup (when in AVR/EDBG mode):
     *  Vendor ID: 0x03eb (1003)
     *  Product ID: 0x2180 (8576)
     */
    class MplabSnap: public DebugTool, public Usb::UsbDevice
    {
    public:
        static const std::uint16_t USB_VENDOR_ID = 1003;
        static const std::uint16_t USB_PRODUCT_ID = 8576;

        MplabSnap(): UsbDevice(MplabSnap::USB_VENDOR_ID, MplabSnap::USB_PRODUCT_ID) {}

        void init() override;

        void close() override;

        Protocols::CmsisDap::Edbg::EdbgInterface& getEdbgInterface()  {
            return this->edbgInterface;
        }

        TargetInterfaces::Microchip::Avr::Avr8::Avr8DebugInterface* getAvr8DebugInterface() override {
            return this->edbgAvr8Interface.get();
        }

        std::string getName() override {
            return "MPLAB Snap";
        };

        /**
         * Retrieves the device serial number via the Discovery Protocol.
         *
         * @return
         */
        std::string getSerialNumber() override;

        /**
         * Starts a session with the EDBG-based tool using the housekeeping protocol.
         */
        void startSession();

        /**
         * Ends the active session with the debug tool.
         */
        void endSession();

    private:
        Protocols::CmsisDap::Edbg::EdbgInterface edbgInterface = Protocols::CmsisDap::Edbg::EdbgInterface();

        /**
         * The MPLAB Snap employs the EDBG AVR8 Generic protocol, for debugging AVR8 targets. This protocol is
         * implemented in EdbgAvr8Interface. See the EdbgAvr8Interface class for more information.
         */
        std::unique_ptr<Protocols::CmsisDap::Edbg::Avr::EdbgAvr8Interface> edbgAvr8Interface = nullptr;

        bool sessionStarted = false;
    };
}
