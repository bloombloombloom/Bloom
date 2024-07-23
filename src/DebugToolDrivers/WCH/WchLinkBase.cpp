#include "WchLinkBase.hpp"

#include "Protocols/WchLink/WchLinkInterface.hpp"

#include "src/TargetController/Exceptions/DeviceInitializationFailure.hpp"

#include "src/Logger/Logger.hpp"

namespace DebugToolDrivers::Wch
{
    using Exceptions::DeviceInitializationFailure;

    WchLinkBase::WchLinkBase(
        WchLinkVariant variant,
        std::uint16_t vendorId,
        std::uint16_t productId,
        std::uint8_t wchLinkUsbInterfaceNumber
    )
        : UsbDevice(vendorId, productId)
        , variant(variant)
        , wchLinkUsbInterfaceNumber(wchLinkUsbInterfaceNumber)
    {}

    void WchLinkBase::init() {
        UsbDevice::init();

        this->detachKernelDriverFromInterface(this->wchLinkUsbInterfaceNumber);

        this->wchLinkUsbInterface = std::make_unique<Usb::UsbInterface>(
            this->wchLinkUsbInterfaceNumber,
            this->libusbDeviceHandle.get()
        );

        this->wchLinkUsbInterface->init();

        this->wchLinkInterface = std::make_unique<Protocols::WchLink::WchLinkInterface>(
            *(this->wchLinkUsbInterface.get()),
            *this
        );

        if (this->getDeviceInfo().variant != this->variant) {
            throw DeviceInitializationFailure{
                "WCH-Link variant mismatch - device returned variant ID that doesn't match the " + this->getName()
                    + " variant ID"
            };
        }

        this->initialised = true;
    }

    void WchLinkBase::close() {
        if (this->wchLinkUsbInterface) {
            this->wchLinkUsbInterface->close();
        }

        this->initialised = false;
    }

    void WchLinkBase::postInit() {
        Logger::info("WCH-Link firmware version: " + this->getDeviceInfo().firmwareVersion.toString());
    }

    bool WchLinkBase::isInitialised() const {
        return this->initialised;
    }

    std::string WchLinkBase::getSerialNumber() {
        return UsbDevice::getSerialNumber();
    }

    ::DebugToolDrivers::Protocols::RiscVDebugSpec::DebugTranslator* WchLinkBase::getRiscVDebugInterface(
        const Targets::RiscV::TargetDescriptionFile& targetDescriptionFile,
        const Targets::RiscV::RiscVTargetConfig& targetConfig
    ) {
        using ::DebugToolDrivers::Protocols::RiscVDebugSpec::DebugTranslator;

        if (!this->wchRiscVTranslator) {
            this->wchRiscVTranslator = std::make_unique<DebugTranslator>(
                *(this->wchLinkInterface.get()),
                targetDescriptionFile,
                targetConfig
            );
        }
        return this->wchRiscVTranslator.get();
    }

    Protocols::WchLink::WchLinkInterface* WchLinkBase::getRiscVProgramInterface(
        const Targets::RiscV::TargetDescriptionFile& targetDescriptionFile,
        const Targets::RiscV::RiscVTargetConfig& targetConfig
    ) {
        return this->wchLinkInterface.get();
    }

    Protocols::WchLink::WchLinkInterface* WchLinkBase::getRiscVIdentificationInterface(
        const Targets::RiscV::TargetDescriptionFile& targetDescriptionFile,
        const Targets::RiscV::RiscVTargetConfig& targetConfig
    ) {
        return this->wchLinkInterface.get();
    }

    const DeviceInfo& WchLinkBase::getDeviceInfo() const {
        if (!this->cachedDeviceInfo.has_value()) {
            this->cachedDeviceInfo = this->wchLinkInterface->getDeviceInfo();
        }

        return *(this->cachedDeviceInfo);
    }
}
