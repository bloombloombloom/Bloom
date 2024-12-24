#pragma once

#include <QWidget>
#include <cstdint>
#include <QVBoxLayout>
#include <QPainter>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetWidgets/TargetPinWidget.hpp"

#include "PinBodyWidget.hpp"

namespace Widgets::InsightTargetWidgets::Dip
{
    enum class Position: std::uint8_t
    {
        TOP,
        BOTTOM
    };

    class PinWidget: public TargetPinWidget
    {
        Q_OBJECT

    public:
        static constexpr int MINIMUM_WIDTH = PinBodyWidget::WIDTH;
        static constexpr int WIDTH_SPACING = 4;
        static constexpr int PIN_LABEL_SPACING = 2;
        static constexpr int PIN_NAME_LABEL_LONG_LINE_LENGTH = 25;
        static constexpr int PIN_NAME_LABEL_SHORT_LINE_LENGTH = 5;
        static constexpr int PIN_DIRECTION_LABEL_LONG_LINE_LENGTH = 22;
        static constexpr int PIN_DIRECTION_LABEL_SHORT_LINE_LENGTH = 21;
        static constexpr int LABEL_HEIGHT = 20;
        static constexpr int MAXIMUM_LABEL_WIDTH = 42;
        static constexpr int MAXIMUM_LABEL_HEIGHT = 20;
        static constexpr int MAXIMUM_HEIGHT = PinBodyWidget::HEIGHT + PinWidget::PIN_LABEL_SPACING
            + PinWidget::LABEL_HEIGHT;

        Position position = Position::TOP;
        QString pinNameLabelText;

        PinWidget(
            const Targets::TargetPinDescriptor& pinDescriptor,
            std::optional<std::reference_wrapper<const Targets::TargetPadDescriptor>> padDescriptor,
            const Targets::TargetPinoutDescriptor& pinoutDescriptor,
            QWidget* parent
        );

        void updatePadState(const Targets::TargetGpioPadState& padState) override {
            TargetPinWidget::updatePadState(padState);

            if (this->bodyWidget != nullptr) {
                this->bodyWidget->setPadState(padState);
            }
        }

    private:
        QVBoxLayout* layout = nullptr;
        Label* pinNumberLabel = nullptr;
        PinBodyWidget* bodyWidget = nullptr;
    };
}
