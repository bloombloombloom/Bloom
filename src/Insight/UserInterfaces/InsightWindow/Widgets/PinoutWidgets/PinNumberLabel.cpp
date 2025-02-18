#include "PinNumberLabel.hpp"

namespace Widgets::PinoutWidgets
{
    PinNumberLabel::PinNumberLabel(const Targets::TargetPinDescriptor& pinDescriptor)
        : pinNumber(QString::fromStdString(pinDescriptor.position))
    {}

    const QString& PinNumberLabel::text() const {
        return this->pinNumber;
    }

    const QColor& PinNumberLabel::rectColor() const {
        static constexpr auto RECT_COLOR = QColor{0x40, 0x53, 0x51};
        return RECT_COLOR;
    }

    const QColor& PinNumberLabel::fontColor() const {
        static constexpr auto FONT_COLOR = QColor{0xA8, 0xA8, 0xA8};
        return FONT_COLOR;
    }
}
