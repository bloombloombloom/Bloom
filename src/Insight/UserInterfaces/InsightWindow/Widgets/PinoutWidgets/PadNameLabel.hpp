#pragma once

#include <functional>
#include <optional>

#include "Label.hpp"

#include "src/Targets/TargetPadDescriptor.hpp"

namespace Widgets::PinoutWidgets
{
    class PadNameLabel: public Label
    {
    public:
        explicit PadNameLabel(std::optional<std::reference_wrapper<const Targets::TargetPadDescriptor>> padDescriptor);

        [[nodiscard]] const QString& text() const override;

    protected:
        QString padName;

        [[nodiscard]] const QColor& rectColor() const override;
        [[nodiscard]] const QColor& fontColor() const override;
    };
}
