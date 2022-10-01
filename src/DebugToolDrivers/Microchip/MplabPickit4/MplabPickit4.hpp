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
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/EdbgAvrIspInterface.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AvrCommandFrames.hpp"

namespace Bloom::DebugToolDrivers
{
    /**
     * Like the MPLAB Snap, the PICkit 4 is a hybrid device. It can present itself as an EDBG (Embedded Debugger)
     * device via a firmware update, issued by Microchip software (the MPLAB IDE and IPE).
     *
     * This debug tool driver currently only supports the device when in AVR mode. Because the device uses different
     * vendor & product IDs depending on the mode, it is easy to determine which is which. In fact, Bloom will not even
     * recognise the device if it's not in AVR mode.
     *
     * USB Setup (when in AVR/EDBG mode):
     *  Vendor ID: 0x03eb (1003)
     *  Product ID: 0x2177 (8567)
     */
    class MplabPickit4: public DebugTool, public Usb::UsbDevice
    {
    public:
        static const std::uint16_t USB_VENDOR_ID = 1003;
        static const std::uint16_t USB_PRODUCT_ID = 8567;

        MplabPickit4();

        void init() override;

        void close() override;

        TargetInterfaces::Microchip::Avr::Avr8::Avr8DebugInterface* getAvr8DebugInterface() override {
            return this->edbgAvr8Interface.get();
        }

        TargetInterfaces::Microchip::Avr::AvrIspInterface* getAvrIspInterface() override {
            return this->edbgAvrIspInterface.get();
        }

        std::string getName() override {
            return "MPLAB PICkit 4";
        }

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
        std::unique_ptr<Protocols::CmsisDap::Edbg::EdbgInterface> edbgInterface = nullptr;
        std::unique_ptr<Protocols::CmsisDap::Edbg::Avr::EdbgAvr8Interface> edbgAvr8Interface = nullptr;
        std::unique_ptr<Protocols::CmsisDap::Edbg::Avr::EdbgAvrIspInterface> edbgAvrIspInterface = nullptr;

        bool sessionStarted = false;
    };
}
