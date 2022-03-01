#pragma once

#include <QWidget>
#include <cstdint>
#include <QBoxLayout>
#include <QLabel>

#include "../TargetPinWidget.hpp"
#include "PinBodyWidget.hpp"
#include "src/Targets/TargetVariant.hpp"

namespace Bloom::Widgets::InsightTargetWidgets::Qfp
{
    enum class Position: std::uint8_t
    {
        TOP,
        BOTTOM,
        LEFT,
        RIGHT,
    };

    class PinWidget: public TargetPinWidget
    {
        Q_OBJECT

    public:
        static constexpr int PIN_WIDGET_LAYOUT_PADDING = 46;
        static constexpr int WIDTH_SPACING = 4;
        static constexpr int PIN_LABEL_LONG_LINE_LENGTH = 25;
        static constexpr int PIN_LABEL_SHORT_LINE_LENGTH = 5;
        static constexpr int PIN_LABEL_SPACING = 2;
        static constexpr int LABEL_HEIGHT = 20;
        static constexpr int MAXIMUM_PIN_NUMBER_LABEL_WIDTH = 26;
        static constexpr int MAXIMUM_PIN_DIRECTION_LABEL_WIDTH = 24;
        static constexpr int MAXIMUM_LABEL_WIDTH = 42;
        static constexpr int MAXIMUM_LABEL_HEIGHT = 20;
        static constexpr int MAXIMUM_HORIZONTAL_WIDTH = PinBodyWidget::HEIGHT + PinWidget::MAXIMUM_PIN_NUMBER_LABEL_WIDTH + 3;
        static constexpr int MAXIMUM_HORIZONTAL_HEIGHT = PinBodyWidget::WIDTH;
        static constexpr int MAXIMUM_VERTICAL_HEIGHT = PinBodyWidget::HEIGHT + PinWidget::LABEL_HEIGHT + 3;
        static constexpr int MAXIMUM_VERTICAL_WIDTH = PinBodyWidget::WIDTH;

        Position position;
        QString pinNameLabelText;

        PinWidget(
            const Targets::TargetPinDescriptor& pinDescriptor,
            const Targets::TargetVariant& targetVariant,
            InsightWorker& insightWorker,
            QWidget* parent
        );

        void updatePinState(const Targets::TargetPinState& pinState) override;

    private:
        QBoxLayout* layout = nullptr;
        QLabel* pinNumberLabel = nullptr;
        QLabel* pinNameLabel = nullptr;
        QLabel* pinDirectionLabel = nullptr;
        PinBodyWidget* bodyWidget = nullptr;
    };
}
