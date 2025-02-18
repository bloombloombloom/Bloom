#include "GpioDisabledLabel.hpp"

namespace Widgets::PinoutWidgets
{
    const QString& GpioDisabledLabel::text() const {
        static const auto TEXT = QString{"GPIO PORT DISABLED"};
        return TEXT;
    }
}
