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
     * The Atmel-ICE device is an EDBG (Embedded Debugger) device. It implements the CMSIS-DAP layer as well
     * as an Atmel Data Gateway Interface (DGI).
     *
     * Communication:
     *  Using the Atmel-ICE device for AVR debugging/programming requires the AVR communication protocol, which
     *  is a sub-protocol of the CMSIS-DAP (AVR communication protocol commands are wrapped in CMSIS-DAP vendor
     *  commands). AVR communication protocol commands contain AVR frames, in which commands for numerous
     *  sub-protocols are enveloped.
     *
     *  So to summarise, issuing an AVR command to the EDBG device, involves:
     *  Actual command data -> AVR sub-protocol -> AVR Frames -> CMSIS-DAP Vendor commands -> EDBG Device
     *
     *  For more information on protocols and sub-protocols employed by Microchip EDBG devices, see
     *  the 'Embedded Debugger-Based Tools Protocols User's Guide' document by Microchip.
     *  @link http://ww1.microchip.com/downloads/en/DeviceDoc/50002630A.pdf
     *
     * USB Setup:
     *  Vendor ID: 0x03eb (1003)
     *  Product ID: 0x2141 (8513)
     *
     *  The Atmel-ICE consists of two USB interfaces. One HID interface for CMSIS-DAP and one vendor specific
     *  interface for the DGI. We only work with the CMSIS-DAP interface, for now.
     */
    class AtmelIce: public DebugTool, public Usb::UsbDevice
    {
    public:
        static const std::uint16_t USB_VENDOR_ID = 1003;
        static const std::uint16_t USB_PRODUCT_ID = 8513;

        AtmelIce(): UsbDevice(AtmelIce::USB_VENDOR_ID, AtmelIce::USB_PRODUCT_ID) {}

        void init() override;

        void close() override;

        Protocols::CmsisDap::Edbg::EdbgInterface& getEdbgInterface()  {
            return this->edbgInterface;
        }

        TargetInterfaces::Microchip::Avr::Avr8::Avr8Interface* getAvr8Interface() override {
            return this->edbgAvr8Interface.get();
        }

        std::string getName() override {
            return "Atmel-ICE";
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
         * Any non-EDBG CMSIS-DAP commands for the Atmel-ICE can be sent through the EdbgInterface (as the
         * EdbgInterface extends the CmsisDapInterface).
         */
        Protocols::CmsisDap::Edbg::EdbgInterface edbgInterface = Protocols::CmsisDap::Edbg::EdbgInterface();

        /**
         * The Atmel-ICE employs the EDBG AVR8 Generic protocol, for debugging AVR8 targets. This protocol is
         * implemented in EdbgAvr8Interface. See the EdbgAvr8Interface class for more information.
         */
        std::unique_ptr<Protocols::CmsisDap::Edbg::Avr::EdbgAvr8Interface> edbgAvr8Interface = nullptr;

        bool sessionStarted = false;
    };
}
