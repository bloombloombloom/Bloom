#include "MplabPickit4.hpp"

#include "src/TargetController/Exceptions/DeviceNotFound.hpp"
#include "src/Services/PathService.hpp"

namespace DebugToolDrivers
{
    MplabPickit4::MplabPickit4()
        : EdbgDevice(
            MplabPickit4::USB_VENDOR_ID,
            MplabPickit4::USB_PRODUCT_ID,
            MplabPickit4::CMSIS_HID_INTERFACE_NUMBER
        )
    {}

    void MplabPickit4::init() {
        using Exceptions::DeviceNotFound;

        try {
            EdbgDevice::init();

        } catch (const DeviceNotFound& exception) {
            /*
             * The MPLAB PICkit 4 could be connected but not in AVR mode - if this is the case, inform the user and
             * direct them to the AVR mode article.
             */
            const auto nonEdbgDevices = this->findMatchingDevices(
                MplabPickit4::NON_EDBG_USB_VENDOR_ID,
                MplabPickit4::NON_EDBG_USB_PRODUCT_ID
            );

            if (!nonEdbgDevices.empty()) {
                throw DeviceNotFound(
                    "The connected MPLAB PICkit 4 device is not in \"AVR mode\". Please follow the instructions at "
                        + Services::PathService::homeDomainName() + "/docs/avr-mode"
                );
            }

            throw exception;
        }
    }

    void MplabPickit4::configureAvr8Interface() {
        this->edbgAvr8Interface->setReactivateJtagTargetPostProgrammingMode(true);
    }
}
