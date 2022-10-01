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
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/EdbgAvrIspInterface.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AvrCommandFrames.hpp"

namespace Bloom::DebugToolDrivers
{
    /**
     * The Xplained Mini is an evaluation board featuring an on-board debugger. The debugger is EDBG-based.
     *
     * Because the on-board debugger is EDBG-based, we can employ the same AVR8 driver implementation as we do with
     * other EDBG-based debuggers. See the EdbgAvr8Interface class for more.
     *
     * USB Setup:
     *  Vendor ID: 0x03eb (1003)
     *  Product ID: 0x2145 (8517)
     */
    class XplainedMini: public DebugTool, public Usb::UsbDevice
    {
    public:
        static const std::uint16_t USB_VENDOR_ID = 1003;
        static const std::uint16_t USB_PRODUCT_ID = 8517;

        XplainedMini();

        void init() override;

        void close() override;

        DebugToolDrivers::TargetInterfaces::TargetPowerManagementInterface* getTargetPowerManagementInterface() override {
            return this->targetPowerManagementInterface.get();
        }

        TargetInterfaces::Microchip::Avr::Avr8::Avr8DebugInterface* getAvr8DebugInterface() override {
            return this->edbgAvr8Interface.get();
        }

        TargetInterfaces::Microchip::Avr::AvrIspInterface* getAvrIspInterface() override {
            return this->edbgAvrIspInterface.get();
        }

        std::string getName() override {
            return "Xplained Mini";
        }

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
        std::unique_ptr<Protocols::CmsisDap::Edbg::EdbgInterface> edbgInterface = nullptr;
        std::unique_ptr<Protocols::CmsisDap::Edbg::Avr::EdbgAvr8Interface> edbgAvr8Interface = nullptr;
        std::unique_ptr<Protocols::CmsisDap::Edbg::Avr::EdbgAvrIspInterface> edbgAvrIspInterface = nullptr;
        std::unique_ptr<
            Protocols::CmsisDap::Edbg::EdbgTargetPowerManagementInterface
        > targetPowerManagementInterface = nullptr;

        bool sessionStarted = false;
    };
}
