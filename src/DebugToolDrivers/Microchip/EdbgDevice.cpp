#include "EdbgDevice.hpp"

#include <array>
#include <thread>
#include <chrono>

#include "src/DebugToolDrivers/Usb/Hid/HidInterface.hpp"
#include "src/DebugToolDrivers/Protocols/CmsisDap/CmsisDapInterface.hpp"
#include "src/DebugToolDrivers/Microchip/Protocols/Edbg/Avr/CommandFrames/AvrCommandFrames.hpp"
#include "src/DebugToolDrivers/Usb/UsbInterface.hpp"

#include "src/Exceptions/InvalidConfig.hpp"
#include "src/TargetController/Exceptions/DeviceFailure.hpp"
#include "src/TargetController/Exceptions/DeviceInitializationFailure.hpp"

namespace DebugToolDrivers::Microchip
{
    using namespace Microchip::Protocols::Edbg::Avr;

    using Exceptions::DeviceFailure;
    using Exceptions::DeviceInitializationFailure;

    EdbgDevice::EdbgDevice(
        const DebugToolConfig& debugToolConfig,
        std::uint16_t vendorId,
        std::uint16_t productId,
        std::uint8_t cmsisHidInterfaceNumber,
        bool supportsTargetPowerManagement,
        std::optional<std::uint8_t> configurationIndex
    )
        : UsbDevice(vendorId, productId)
        , toolConfig(EdbgToolConfig{debugToolConfig})
        , cmsisHidInterfaceNumber(cmsisHidInterfaceNumber)
        , supportsTargetPowerManagement(supportsTargetPowerManagement)
        , configurationIndex(configurationIndex)
    {}

    void EdbgDevice::init() {
        using ::DebugToolDrivers::Protocols::CmsisDap::CmsisDapInterface;
        using Microchip::Protocols::Edbg::EdbgInterface;
        using Microchip::Protocols::Edbg::EdbgTargetPowerManagementInterface;

        UsbDevice::init();

        this->detachKernelDriverFromInterface(this->cmsisHidInterfaceNumber);

        if (this->configurationIndex.has_value()) {
            this->setConfiguration(this->configurationIndex.value());
        }

        auto cmsisHidInterface = Usb::HidInterface{
            this->cmsisHidInterfaceNumber,
            this->getEndpointMaxPacketSize(
                this->getFirstEndpointAddress(this->cmsisHidInterfaceNumber, LIBUSB_ENDPOINT_IN)
            ),
            this->vendorId,
            this->productId
        };

        cmsisHidInterface.init();

        this->edbgInterface = std::make_unique<EdbgInterface>(std::move(cmsisHidInterface));

        /*
         * Sometimes, some EDBG tools misbehave when we send commands too quickly, even though we wait for a response
         * for each command we send...
         *
         * Because of this, we make available the option to enforce a minimum time gap between commands. This prevents
         * the tool from misbehaving, but effectively chokes the EDBG driver, causing a drag on Bloom's performance.
         *
         * The command delay is disabled by default for all EDBG tools, but the user can enable it via their project
         * config.
         */
        if (this->toolConfig.cmsisCommandDelay.has_value()) {
            const auto& cmsisCommandDelay = *(this->toolConfig.cmsisCommandDelay);

            if (cmsisCommandDelay > CmsisDapInterface::CMSIS_COMMAND_DELAY_MAX) {
                throw Exceptions::InvalidConfig{
                    "CMSIS command delay value (" + std::to_string(cmsisCommandDelay.count())
                        + " ms) is too high. Maximum value: " + std::to_string(
                            CmsisDapInterface::CMSIS_COMMAND_DELAY_MAX.count()
                        ) + " ms"
                };
            }

            this->edbgInterface->setCommandDelay(cmsisCommandDelay);
        }

        // We don't need to claim the CMSISDAP interface here as the HIDAPI will have already done so.
        if (!this->sessionStarted) {
            this->startSession();
        }

        if (this->supportsTargetPowerManagement) {
            this->targetPowerManagementInterface = std::make_unique<EdbgTargetPowerManagementInterface>(
                this->edbgInterface.get()
            );
        }

        this->initialised = true;
    }

    void EdbgDevice::close() {
        if (this->sessionStarted) {
            this->endSession();
        }

        this->edbgInterface->getUsbHidInterface().close();
        UsbDevice::close();
        this->initialised = false;
    }

    void EdbgDevice::postInit() {
        // TODO: Log firmware version of EDBG device
    }

    bool EdbgDevice::isInitialised() const {
        return this->initialised;
    }

    std::string EdbgDevice::getSerialNumber() {
        using namespace CommandFrames::Discovery;
        using ResponseFrames::Discovery::ResponseId;

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            Query{QueryContext::SERIAL_NUMBER}
        );

        if (responseFrame.id != ResponseId::OK) {
            throw DeviceInitializationFailure{
                "Failed to fetch serial number from device - invalid Discovery Protocol response ID."
            };
        }

        const auto data = responseFrame.getPayloadData();
        return std::string{data.begin(), data.end()};
    }

    DebugToolDrivers::TargetInterfaces::TargetPowerManagementInterface* EdbgDevice::getTargetPowerManagementInterface()
    {
        return this->targetPowerManagementInterface.get();
    }

    TargetInterfaces::Microchip::Avr8::Avr8DebugInterface* EdbgDevice::getAvr8DebugInterface(
        const Targets::Microchip::Avr8::TargetDescriptionFile& targetDescriptionFile,
        const Targets::Microchip::Avr8::Avr8TargetConfig& targetConfig
    ) {
        if (this->edbgAvr8Interface == nullptr) {
            this->edbgAvr8Interface = std::make_unique<EdbgAvr8Interface>(
                this->edbgInterface.get(),
                targetDescriptionFile,
                targetConfig
            );

            this->configureAvr8Interface();
        }

        return this->edbgAvr8Interface.get();
    }

    TargetInterfaces::Microchip::Avr8::AvrIspInterface* EdbgDevice::getAvrIspInterface(
        const Targets::Microchip::Avr8::TargetDescriptionFile& targetDescriptionFile,
        const Targets::Microchip::Avr8::Avr8TargetConfig& targetConfig
    ) {
        if (this->edbgAvrIspInterface == nullptr) {
            this->edbgAvrIspInterface = std::make_unique<EdbgAvrIspInterface>(
                this->edbgInterface.get(),
                targetDescriptionFile
            );

            this->configureAvr8Interface();
        }

        return this->edbgAvrIspInterface.get();
    }

    void EdbgDevice::startSession() {
        using namespace CommandFrames::HouseKeeping;
        using ResponseFrames::HouseKeeping::ResponseId;

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(StartSession{});
        if (responseFrame.id == ResponseId::FAILED) {
            // Failed response returned!
            throw DeviceInitializationFailure{"Failed to start session with EDBG device!"};
        }

        this->sessionStarted = true;
    }

    void EdbgDevice::endSession() {
        using namespace CommandFrames::HouseKeeping;
        using ResponseFrames::HouseKeeping::ResponseId;

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(EndSession{});
        if (responseFrame.id == ResponseId::FAILED) {
            // Failed response returned!
            throw DeviceFailure{"Failed to end session with EDBG device!"};
        }

        this->sessionStarted = false;
    }

    void EdbgDevice::exitBootloaderMode(UsbDevice& device) const {
        static constexpr auto INTERFACE_NUMBER = std::uint8_t{0};
        static constexpr auto COMMAND_ENDPOINT_ADDRESS = std::uint8_t{0x02};

        auto interface = Usb::UsbInterface{INTERFACE_NUMBER, device.libusbDeviceHandle.get()};
        interface.init();

        static constexpr auto EXIT_BL_MODE_COMMAND = std::to_array<unsigned char>({0xE6});

        interface.writeBulk(COMMAND_ENDPOINT_ADDRESS, EXIT_BL_MODE_COMMAND, 64);
    }

    void EdbgDevice::enableEdbgMode(UsbDevice& device) const {
        static constexpr auto INTERFACE_NUMBER = std::uint8_t{0};
        static constexpr auto COMMAND_ENDPOINT_ADDRESS = std::uint8_t{0x02};

        auto interface = Usb::UsbInterface{INTERFACE_NUMBER, device.libusbDeviceHandle.get()};
        interface.init();

        static constexpr auto AVR_MODE_COMMAND = std::to_array<unsigned char>({0xF0, 0x01});
        static constexpr auto RESET_COMMAND = std::to_array<unsigned char>({0xED});

        interface.writeBulk(COMMAND_ENDPOINT_ADDRESS, AVR_MODE_COMMAND, 64);
        std::this_thread::sleep_for(std::chrono::milliseconds{250});
        interface.writeBulk(COMMAND_ENDPOINT_ADDRESS, RESET_COMMAND, 64);
    }
}
