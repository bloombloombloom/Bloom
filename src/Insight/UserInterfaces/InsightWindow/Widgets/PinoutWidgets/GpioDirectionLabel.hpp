#pragma once

#include "Label.hpp"

#include "src/Targets/TargetGpioPadState.hpp"

namespace Widgets::PinoutWidgets
{
    class GpioDirectionLabel: public Label
    {
    public:
        explicit GpioDirectionLabel(const Targets::TargetGpioPadState& padeState);
        [[nodiscard]] const QString& text() const override;

    protected:
        const Targets::TargetGpioPadState& padeState;
    };
}
