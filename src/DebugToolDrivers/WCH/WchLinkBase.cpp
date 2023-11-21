#include "WchLinkBase.hpp"

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
            *(this->wchLinkUsbInterface.get())
        );

        if (this->getDeviceInfo().variant != this->variant) {
            throw DeviceInitializationFailure(
                "WCH-Link variant mismatch - device returned variant ID that doesn't match the " + this->getName()
                    + " variant ID"
            );
        }

        this->setInitialised(true);
    }

    void WchLinkBase::close() {
        if (this->wchLinkUsbInterface) {
            this->wchLinkUsbInterface->close();
        }
    }

    std::string WchLinkBase::getSerialNumber() {
        return UsbDevice::getSerialNumber();
    }

    std::string WchLinkBase::getFirmwareVersionString() {
        return "v" + this->getDeviceInfo().firmwareVersion.toString();
    }

    const DeviceInfo& WchLinkBase::getDeviceInfo() const {
        if (!this->cachedDeviceInfo.has_value()) {
            this->cachedDeviceInfo = this->wchLinkInterface->getDeviceInfo();
        }

        return *(this->cachedDeviceInfo);
    }
}
