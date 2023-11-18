#pragma once

#include <cstdint>
#include <optional>
#include <memory>

#include "src/DebugToolDrivers/DebugTool.hpp"
#include "src/DebugToolDrivers/USB/UsbDevice.hpp"

#include "Protocols/EDBG/EdbgInterface.hpp"
#include "Protocols/EDBG/AVR/EdbgAvr8Interface.hpp"
#include "Protocols/EDBG/AVR/EdbgAvrIspInterface.hpp"
#include "Protocols/EDBG/EdbgTargetPowerManagementInterface.hpp"

namespace DebugToolDrivers::Microchip
{
    /**
     * Microchip EDBG (Embedded Debugger) devices implement the CMSIS-DAP interface. As well as the CMSIS-DAP protocol,
     * they support communication protocols that are specific to Microchip and enable the debugging and programming
     * of AVR microcontrollers. These protocols are sub-protocols of the CMSIS-DAP protocol (they are implemented as
     * CMSIS-DAP vendor commands).
     *
     * For more information on protocols and sub-protocols employed by Microchip EDBG devices, see
     * the 'Embedded Debugger-Based Tools Protocols User's Guide' document by Microchip.
     * @link http://ww1.microchip.com/downloads/en/DeviceDoc/50002630A.pdf
     *
     * This class serves as an abstract base class for all Microchip EDBG devices supported by Bloom.
     */
    class EdbgDevice: public DebugTool, public Usb::UsbDevice
    {
    public:
        EdbgDevice(
            std::uint16_t vendorId,
            std::uint16_t productId,
            std::uint8_t cmsisHidInterfaceNumber,
            bool supportsTargetPowerManagement = false,
            std::optional<std::uint8_t> configurationIndex = std::nullopt
        );

        /**
         * Will attempt to locate the EDBG (USB) device and claim the CMSIS-DAP USB interface.
         *
         * Upon claiming the USB interface, a connection will be established and a session will be started.
         */
        void init() override;

        /**
         * Terminates any active session with the device and releases the CMSIS-DAP USB interface, as well as any
         * other USB device resources.
         */
        void close() override;

        TargetInterfaces::Microchip::Avr::Avr8::Avr8DebugInterface* getAvr8DebugInterface(
            const Targets::Microchip::Avr::Avr8Bit::Avr8TargetConfig& targetConfig,
            Targets::Microchip::Avr::Avr8Bit::Family targetFamily,
            const Targets::Microchip::Avr::Avr8Bit::TargetParameters& targetParameters,
            const Targets::TargetRegisterDescriptorMapping& targetRegisterDescriptorsById
        ) override;

        TargetInterfaces::Microchip::Avr::AvrIspInterface* getAvrIspInterface(
            const Targets::Microchip::Avr::Avr8Bit::Avr8TargetConfig& targetConfig
        ) override {
            return this->edbgAvrIspInterface.get();
        }

        DebugToolDrivers::TargetInterfaces::TargetPowerManagementInterface* getTargetPowerManagementInterface() override {
            return this->targetPowerManagementInterface.get();
        }

        /**
         * Retrieves the device serial number via the "Discovery" EDBG sub-protocol.
         *
         * @return
         */
        std::string getSerialNumber() override;

        /**
         * Retrieves the EDBG firmware version.
         *
         * @return
         */
        std::string getFirmwareVersionString() override;

        /**
         * Starts a session with the EDBG device using the "Housekeeping" EDBG sub-protocol.
         */
        void startSession();

        /**
         * Ends the active session with the device.
         */
        void endSession();

    protected:
        /**
         * The USB interface number of the CMSIS-DAP HID interface.
         */
        std::uint8_t cmsisHidInterfaceNumber;

        /**
         * Some EDBG devices provide the means to manage target power. This functionality would typically be found on
         * AVR evaluation boards with an on-board EDBG-based debugger (like the Xplained Mini or Curiosity Nano boards).
         *
         * If EdbgDevice::supportsTargetPowerManagement is set to true, we will instantiate an instance of the
         * EdbgTargetPowerManagementInterface class upon device initialisation. The interface is exposed via
         * the EdbgDevice::getTargetPowerManagementInterface() member function.
         */
        bool supportsTargetPowerManagement = false;

        /**
         * The USB configuration index to set on the EDBG device before attempting to claim the CMSIS-DAP USB
         * interface.
         *
         * If not specified, we won't attempt to modify the USB configuration at all.
         */
        std::optional<std::uint8_t> configurationIndex = std::nullopt;

        /**
         * The EdbgInterface class provides the ability to communicate with the EDBG device, using any of the EDBG
         * sub-protocols.
         */
        std::unique_ptr<Microchip::Protocols::Edbg::EdbgInterface> edbgInterface = nullptr;

        /**
         * The EdbgAvr8Interface class implements the AVR8 Generic EDBG sub-protocol. This protocol is used to perform
         * debugging and programming operations on 8-bit AVR targets.
         *
         * The class implements the Avr8DebugInterface.
         */
        std::unique_ptr<Microchip::Protocols::Edbg::Avr::EdbgAvr8Interface> edbgAvr8Interface = nullptr;

        /**
         * The EdbgAvrIspInterface class implements the AVRISP EDBG sub-protocol, for interfacing with AVR targets via
         * their ISP physical interface.
         *
         * ISP cannot be used for debugging operations. The EdbgAvrIspInterface class does *not* implement
         * the Avr8DebugInterface.
         *
         * Currently, Bloom will only use the ISP interface as a fallback when attempting to connect to debugWire
         * targets. We use the interface to inspect and update the "debugWire enable" (DWEN) fuse-bit, before making a
         * second connection attempt via the debugWire interface.
         */
        std::unique_ptr<Microchip::Protocols::Edbg::Avr::EdbgAvrIspInterface> edbgAvrIspInterface = nullptr;

        /**
         * The EdbgTargetPowerManagementInterface class implements the "EDBG Control" (EDBG_CTRL) sub-protocol, to
         * provide a means to manage the connected target's input power.
         *
         * The class implements the TargetPowerManagementInterface and is exposed via the
         * EdbgDevice::getTargetPowerManagementInterface() member function.
         */
        std::unique_ptr<
            Microchip::Protocols::Edbg::EdbgTargetPowerManagementInterface
        > targetPowerManagementInterface = nullptr;

        bool sessionStarted = false;

        /**
         * Because EDBG devices require fixed-length reports to be transmitted to/from their HID interface, we must
         * know the HID report size (as it differs across EDBG devices).
         *
         * This member function will obtain the report size from the endpoint descriptor of the IN endpoint, from the
         * HID interface.
         *
         * @return
         */
        std::uint16_t getCmsisHidReportSize();

        virtual void configureAvr8Interface() {
            return;
        }
    };
}
