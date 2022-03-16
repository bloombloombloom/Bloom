#pragma once

#include <cstdint>
#include <vector>
#include <memory>

#include "src/DebugToolDrivers/DebugTool.hpp"
#include "src/DebugToolDrivers/USB/UsbDevice.hpp"
#include "src/DebugToolDrivers/USB/HID/HidInterface.hpp"

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/CmsisDapInterface.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/EdbgInterface.hpp"

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/EdbgTargetPowerManagementInterface.hpp"

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/EdbgAvr8Interface.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AvrCommandFrames.hpp"

namespace Bloom::DebugToolDrivers
{
    /**
     * The Xplained Nano is an evaluation board featuring an on-board debugger. The debugger is EDBG-based.
     *
     * Because the on-board debugger is EDBG-based, we can employ the same AVR8 driver implementation as we do with
     * other EDBG-based debuggers. See the EdbgAvr8Interface class for more.
     *
     * USB Setup:
     *  Vendor ID: 0x03eb (1003)
     *  Product ID: 0x2145 (8517)
     */
    class XplainedNano: public DebugTool, public Usb::UsbDevice
    {
    public:
        static const std::uint16_t USB_VENDOR_ID = 1003;
        static const std::uint16_t USB_PRODUCT_ID = 8517;

        XplainedNano(): UsbDevice(XplainedNano::USB_VENDOR_ID, XplainedNano::USB_PRODUCT_ID) {}

        void init() override;

        void close() override;

        Protocols::CmsisDap::Edbg::EdbgInterface& getEdbgInterface()  {
            return this->edbgInterface;
        }

        DebugToolDrivers::TargetInterfaces::TargetPowerManagementInterface* getTargetPowerManagementInterface() override {
            return this->targetPowerManagementInterface.get();
        }

        TargetInterfaces::Microchip::Avr::Avr8::Avr8DebugInterface* getAvr8DebugInterface() override {
            return this->edbgAvr8Interface.get();
        }

        std::string getName() override {
            return "Xplained Nano";
        };

        /**
         * Retrieves the device serial number via the Discovery Protocol.
         *
         * @return
         */
        std::string getSerialNumber() override;

        /**
         * Starts a session with the EDBG-based tool using the Housekeeping protocol.
         */
        void startSession();

        /**
         * Ends the active session with the debug tool.
         */
        void endSession();

    private:
        Protocols::CmsisDap::Edbg::EdbgInterface edbgInterface = Protocols::CmsisDap::Edbg::EdbgInterface();
        std::unique_ptr<Protocols::CmsisDap::Edbg::Avr::EdbgAvr8Interface> edbgAvr8Interface = nullptr;
        std::unique_ptr<
            Protocols::CmsisDap::Edbg::EdbgTargetPowerManagementInterface
        > targetPowerManagementInterface = nullptr;

        bool sessionStarted = false;
    };
}
