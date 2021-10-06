#pragma once

#include <QWidget>
#include <QEvent>

#include "../TargetPinBodyWidget.hpp"

#include "src/Targets/TargetPinDescriptor.hpp"

namespace Bloom::Widgets::InsightTargetWidgets::Dip
{
    class PinBodyWidget: public TargetPinBodyWidget
    {
        Q_OBJECT

    public:
        static const int WIDTH = 30;
        static const int HEIGHT = 40;

        PinBodyWidget(
            QWidget* parent,
            Targets::TargetPinDescriptor pinDescriptor
        ): TargetPinBodyWidget(parent, std::move(pinDescriptor)) {
            this->setFixedSize(PinBodyWidget::WIDTH, PinBodyWidget::HEIGHT);
            this->setObjectName("target-pin-body");
        }

    protected:
        void paintEvent(QPaintEvent* event) override;
        void drawWidget(QPainter& painter);
    };
}
