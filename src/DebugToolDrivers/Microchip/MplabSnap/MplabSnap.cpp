#include "MplabSnap.hpp"

#include "src/TargetController/Exceptions/DeviceNotFound.hpp"
#include "src/Services/PathService.hpp"

namespace DebugToolDrivers::Microchip
{
    MplabSnap::MplabSnap()
        : EdbgDevice(
            MplabSnap::USB_VENDOR_ID,
            MplabSnap::USB_PRODUCT_ID,
            MplabSnap::CMSIS_HID_INTERFACE_NUMBER
        )
    {}

    void MplabSnap::init() {
        using Exceptions::DeviceNotFound;

        try {
            EdbgDevice::init();

        } catch (const DeviceNotFound& exception) {
            /*
             * The MPLAB Snap could be connected but not in AVR mode - if this is the case, inform the user and direct
             * them to the AVR mode article.
             */
            auto nonEdbgDevices = this->findMatchingDevices(
                MplabSnap::NON_EDBG_USB_VENDOR_ID,
                MplabSnap::NON_EDBG_USB_PRODUCT_ID
            );

            if (nonEdbgDevices.empty()) {
                // The MPLAB Snap sometimes uses another product ID when not in AVR mode.
                nonEdbgDevices = this->findMatchingDevices(
                    MplabSnap::NON_EDBG_USB_VENDOR_ID,
                    MplabSnap::NON_EDBG_USB_PRODUCT_ID_ALTERNATIVE
                );
            }

            if (!nonEdbgDevices.empty()) {
                throw DeviceNotFound(
                    "The connected MPLAB Snap device is not in \"AVR mode\". Please follow the instructions at "
                        + Services::PathService::homeDomainName() + "/docs/avr-mode"
                );
            }

            throw exception;
        }
    }

    void MplabSnap::configureAvr8Interface() {
        this->edbgAvr8Interface->setReactivateJtagTargetPostProgrammingMode(true);
    }
}
