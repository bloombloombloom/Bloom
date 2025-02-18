#pragma once

#include "VerticalLabelGroup.hpp"
#include "Pin.hpp"

namespace Widgets::PinoutWidgets
{
    struct VerticalLabelGroupPinPair
    {
        VerticalLabelGroup* labelGroup;
        Pin* pin;
    };
}
