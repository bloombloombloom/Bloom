#pragma once

#include "Label.hpp"

#include "src/Targets/TargetPinDescriptor.hpp"

namespace Widgets::PinoutWidgets
{
    class PinNumberLabel: public Label
    {
    public:
        explicit PinNumberLabel(const Targets::TargetPinDescriptor& pinDescriptor);

        [[nodiscard]] const QString& text() const override;

    protected:
        QString pinNumber;

        [[nodiscard]] const QColor& rectColor() const override;
        [[nodiscard]] const QColor& fontColor() const override;
    };
}
