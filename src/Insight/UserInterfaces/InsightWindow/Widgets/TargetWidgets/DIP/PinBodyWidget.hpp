#pragma once

#include <QWidget>
#include <QEvent>

#include "../TargetPinBodyWidget.hpp"

#include "src/Targets/TargetPinDescriptor.hpp"

namespace Widgets::InsightTargetWidgets::Dip
{
    class PinBodyWidget: public TargetPinBodyWidget
    {
        Q_OBJECT

    public:
        static const int WIDTH = 19;
        static const int HEIGHT = 28;

        PinBodyWidget(
            const Targets::TargetPinDescriptor& pinDescriptor,
            std::optional<std::reference_wrapper<const Targets::TargetPadDescriptor>> padDescriptor,
            QWidget* parent
        );

    protected:
        void paintEvent(QPaintEvent* event) override;
        void drawWidget(QPainter& painter);
    };
}
