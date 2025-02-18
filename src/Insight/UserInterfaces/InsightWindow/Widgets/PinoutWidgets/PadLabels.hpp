#pragma once

#include "GpioDirectionLabel.hpp"
#include "GpioStateLabel.hpp"
#include "GpioDisabledLabel.hpp"

namespace Widgets::PinoutWidgets
{
    struct PadLabels
    {
        GpioDirectionLabel* gpioDirection = nullptr;
        GpioStateLabel* gpioState = nullptr;
        GpioDisabledLabel* gpioDisabled = nullptr;

        void disableGpioLabels() const;
    };
}
