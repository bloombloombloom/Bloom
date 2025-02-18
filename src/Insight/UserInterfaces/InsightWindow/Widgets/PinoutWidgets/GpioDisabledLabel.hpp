#pragma once

#include "Label.hpp"

namespace Widgets::PinoutWidgets
{
    class GpioDisabledLabel: public Label
    {
    public:
        [[nodiscard]] const QString& text() const override;
    };
}
