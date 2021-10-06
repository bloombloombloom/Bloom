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
     * The Power Debugger device is very similar to the Atmel-ICE. It implements the CMSIS-DAP layer as well
     * as an Atmel Data Gateway Interface (DGI).
     *
     * Communication:
     *  Uses the same EDBG protocol as described in the AtmelIce driver. See the AtmelIce debug tool class for more.
     *
     * USB Setup:
     *  Vendor ID: 0x03eb (1003)
     *  Product ID: 0x2141 (8513)
     */
    class PowerDebugger: public DebugTool, public Usb::UsbDevice
    {
    public:
        static const std::uint16_t USB_VENDOR_ID = 1003;
        static const std::uint16_t USB_PRODUCT_ID = 8516;

        PowerDebugger(): UsbDevice(PowerDebugger::USB_VENDOR_ID, PowerDebugger::USB_PRODUCT_ID) {}

        void init() override;

        void close() override;

        Protocols::CmsisDap::Edbg::EdbgInterface& getEdbgInterface()  {
            return this->edbgInterface;
        }

        TargetInterfaces::Microchip::Avr::Avr8::Avr8Interface* getAvr8Interface() override {
            return this->edbgAvr8Interface.get();
        }

        std::string getName() override {
            return "Power Debugger";
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
        /**
         * The EDBG interface implements additional functionality via vendor specific CMSIS-DAP commands.
         * In other words, all EDBG commands are just CMSIS-DAP vendor commands that allow the debug tool
         * to support additional functionality, like AVR programming and debugging.
         *
         * Any non-EDBG CMSIS-DAP commands for the Power Debugger can be sent through the EDBGInterface (as the
         * EdbgInterface extends the CmsisDapInterface).
         */
        Protocols::CmsisDap::Edbg::EdbgInterface edbgInterface = Protocols::CmsisDap::Edbg::EdbgInterface();

        /**
         * The Power Debugger employs the EDBG AVR8Generic protocol for interfacing with AVR8 targets.
         */
        std::unique_ptr<Protocols::CmsisDap::Edbg::Avr::EdbgAvr8Interface> edbgAvr8Interface;

        bool sessionStarted = false;
    };
}
