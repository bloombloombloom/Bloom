#pragma once

#include "HorizontalLabelGroup.hpp"
#include "Pin.hpp"

namespace Widgets::PinoutWidgets
{
    struct HorizontalLabelGroupPinPair
    {
        HorizontalLabelGroup* labelGroup;
        Pin* pin;
    };
}
