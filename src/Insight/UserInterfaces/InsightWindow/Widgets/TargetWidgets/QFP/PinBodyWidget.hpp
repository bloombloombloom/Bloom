#pragma once

#include <QWidget>
#include <QMouseEvent>
#include <utility>

#include "../TargetPinBodyWidget.hpp"

#include "src/Targets/TargetPinDescriptor.hpp"

namespace Widgets::InsightTargetWidgets::Qfp
{
    class PinBodyWidget: public TargetPinBodyWidget
    {
        Q_OBJECT

    public:
        static const int WIDTH = 17;
        static const int HEIGHT = 26;

        PinBodyWidget(QWidget* parent, Targets::TargetPinDescriptor pinDescriptor, bool isVertical);

    protected:
        void paintEvent(QPaintEvent* event) override;
        void drawWidget(QPainter& painter);

    private:
        bool isVertical = false;
    };
}
