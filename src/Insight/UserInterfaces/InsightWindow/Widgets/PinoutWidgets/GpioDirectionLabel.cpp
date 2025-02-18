#include "GpioDirectionLabel.hpp"

namespace Widgets::PinoutWidgets
{
    GpioDirectionLabel::GpioDirectionLabel(const Targets::TargetGpioPadState& padeState)
        : padeState(padeState)
    {}

    const QString& GpioDirectionLabel::text() const {
        static const auto INPUT_TEXT = QString{"INPUT"};
        static const auto OUTPUT_TEXT = QString{"OUTPUT"};

        return this->padeState.direction == Targets::TargetGpioPadState::DataDirection::INPUT
            ? INPUT_TEXT
            : OUTPUT_TEXT;
    }
}
