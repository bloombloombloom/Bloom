#include "GpioStateLabel.hpp"

namespace Widgets::PinoutWidgets
{
    GpioStateLabel::GpioStateLabel(const Targets::TargetGpioPadState& padeState)
        : padeState(padeState)
    {}

    const QString& GpioStateLabel::text() const {
        static const auto HIGH_TEXT = QString{"HIGH"};
        static const auto LOW_TEXT = QString{"LOW"};

        return this->padeState.value == Targets::TargetGpioPadState::State::HIGH
            ? HIGH_TEXT
            : LOW_TEXT;
    }

    const QColor& GpioStateLabel::rectColor() const {
        static constexpr auto RECT_COLOR = QColor{0x52, 0x46, 0x52};
        return RECT_COLOR;
    }
}
