#include "PadNameLabel.hpp"

namespace Widgets::PinoutWidgets
{
    PadNameLabel::PadNameLabel(std::optional<std::reference_wrapper<const Targets::TargetPadDescriptor>> padDescriptor)
        : padName(
            padDescriptor.has_value()
                ? QString::fromStdString(padDescriptor->get().name)
                : "NOT CONNECTED"
        )
    {}

    const QString& PadNameLabel::text() const {
        return this->padName;
    }

    const QColor& PadNameLabel::rectColor() const {
        static constexpr auto RECT_COLOR = QColor{0x38, 0x46, 0x45};
        return RECT_COLOR;
    }

    const QColor& PadNameLabel::fontColor() const {
        static constexpr auto FONT_COLOR = QColor{0xA0, 0xA0, 0xA0};
        return FONT_COLOR;
    }
}
