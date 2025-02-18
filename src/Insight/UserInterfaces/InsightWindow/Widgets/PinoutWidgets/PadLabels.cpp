#include "PadLabels.hpp"

namespace Widgets::PinoutWidgets
{
    void PadLabels::disableGpioLabels() const {
        if (this->gpioDirection != nullptr) {
            this->gpioDirection->enabled = false;
        }

        if (this->gpioState != nullptr) {
            this->gpioState->enabled = false;
        }

        if (this->gpioDisabled != nullptr) {
            this->gpioDisabled->enabled = false;
        }
    }
}
