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

        PinBodyWidget(
            const Targets::TargetPinDescriptor& pinDescriptor,
            std::optional<std::reference_wrapper<const Targets::TargetPadDescriptor>> padDescriptor,
            bool isVertical,
            QWidget* parent
        );

    protected:
        void paintEvent(QPaintEvent* event) override;
        void drawWidget(QPainter& painter);

    private:
        bool isVertical = false;
    };
}
