#pragma once

#include "Label.hpp"

#include "src/Targets/TargetGpioPadState.hpp"

namespace Widgets::PinoutWidgets
{
    class GpioStateLabel: public Label
    {
    public:
        explicit GpioStateLabel(const Targets::TargetGpioPadState& padeState);
        [[nodiscard]] const QString& text() const override;

    protected:
        const Targets::TargetGpioPadState& padeState;

        [[nodiscard]] const QColor& rectColor() const override;
    };
}
