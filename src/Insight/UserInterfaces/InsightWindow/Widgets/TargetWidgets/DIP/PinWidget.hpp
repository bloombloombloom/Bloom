#pragma once

#include <QWidget>
#include <cstdint>
#include <QVBoxLayout>
#include <QLabel>
#include <QPainter>

#include "../TargetPinWidget.hpp"

#include "PinBodyWidget.hpp"
#include "src/Targets/TargetVariant.hpp"

namespace Bloom::Widgets::InsightTargetWidgets::Dip
{
    enum Position: std::uint8_t
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
        static constexpr int PIN_LABEL_LONG_LINE_LENGTH = 25;
        static constexpr int PIN_LABEL_SHORT_LINE_LENGTH = 5;
        static constexpr int MAXIMUM_LABEL_COUNT = 3;
        static constexpr int LABEL_HEIGHT = 20;
        static constexpr int MAXIMUM_PIN_NUMBER_LABEL_WIDTH = 26;
        static constexpr int MAXIMUM_PIN_DIRECTION_LABEL_WIDTH = 24;
        static constexpr int MAXIMUM_LABEL_WIDTH = 42;
        static constexpr int MAXIMUM_LABEL_HEIGHT = 20;
        static constexpr int MAXIMUM_HEIGHT = PinBodyWidget::HEIGHT + PinWidget::PIN_LABEL_SPACING
            + PinWidget::LABEL_HEIGHT;

        Position position = Position::TOP;
        QString pinNameLabelText;

        PinWidget(
            const Targets::TargetPinDescriptor& pinDescriptor,
            const Targets::TargetVariant& targetVariant,
            InsightWorker& insightWorker,
            QWidget* parent
        );

        void updatePinState(const Targets::TargetPinState& pinState) override {
            TargetPinWidget::updatePinState(pinState);

            if (this->bodyWidget != nullptr) {
                this->bodyWidget->setPinState(pinState);
            }
        }

    private:
        QVBoxLayout* layout = nullptr;
        QLabel* pinNumberLabel = nullptr;
        PinBodyWidget* bodyWidget = nullptr;
    };
}
