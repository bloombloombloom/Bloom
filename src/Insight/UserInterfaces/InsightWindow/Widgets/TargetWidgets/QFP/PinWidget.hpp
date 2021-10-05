#pragma once

#include <QWidget>
#include <QBoxLayout>
#include <QLabel>

#include "../TargetPinWidget.hpp"
#include "PinBodyWidget.hpp"
#include "src/Targets/TargetVariant.hpp"

namespace Bloom::Widgets::InsightTargetWidgets::Qfp
{
    class PinWidget: public TargetPinWidget
    {
    Q_OBJECT
    private:
        QBoxLayout* layout = nullptr;
        QLabel* pinNumberLabel = nullptr;
        QLabel* pinNameLabel = nullptr;
        QLabel* pinDirectionLabel = nullptr;
        PinBodyWidget* bodyWidget = nullptr;

        bool isLeftLayout = false;
        bool isBottomLayout = false;
        bool isRightLayout = false;
        bool isTopLayout = false;

        void setLabelColor(const QString& hexColor);

    public:
        static const int PIN_WIDGET_LAYOUT_PADDING = 46;
        static const int WIDTH_SPACING = 8;
        static const int MAXIMUM_LABEL_COUNT = 3;
        static const int LABEL_HEIGHT = 20;
        static const int MAXIMUM_LABEL_WIDTH = PinBodyWidget::WIDTH;
        static const int MAXIMUM_LABEL_HEIGHT = 20;
        static const int MAXIMUM_HORIZONTAL_WIDTH = PinBodyWidget::HEIGHT
            + ((PinWidget::MAXIMUM_LABEL_WIDTH + 12) * (PinWidget::MAXIMUM_LABEL_COUNT - 1)) - 12;
        static const int MAXIMUM_HORIZONTAL_HEIGHT = PinBodyWidget::WIDTH;
        static const int MAXIMUM_VERTICAL_HEIGHT = PinBodyWidget::HEIGHT
            + ((PinWidget::MAXIMUM_LABEL_HEIGHT + 8) * PinWidget::MAXIMUM_LABEL_COUNT) - 8;
        static const int MAXIMUM_VERTICAL_WIDTH = PinBodyWidget::WIDTH;

        PinWidget(
            const Targets::TargetPinDescriptor& pinDescriptor,
            const Targets::TargetVariant& targetVariant,
            InsightWorker& insightWorker,
            QWidget* parent
        );

        void updatePinState(const Targets::TargetPinState& pinState) override;
    };
}
