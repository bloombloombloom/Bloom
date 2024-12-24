#include "PinBodyWidget.hpp"

#include <QPainter>

namespace Widgets::InsightTargetWidgets::Dip
{
    using namespace Targets;

    PinBodyWidget::PinBodyWidget(
        const Targets::TargetPinDescriptor& pinDescriptor,
        std::optional<std::reference_wrapper<const Targets::TargetPadDescriptor>> padDescriptor,
        QWidget* parent
    )
        : TargetPinBodyWidget(
            pinDescriptor,
            padDescriptor,
            parent
        )
    {
        this->setFixedSize(PinBodyWidget::WIDTH, PinBodyWidget::HEIGHT);
    }

    void PinBodyWidget::paintEvent(QPaintEvent* event) {
        auto painter = QPainter{this};
        this->drawWidget(painter);
    }

    void PinBodyWidget::drawWidget(QPainter& painter) {
        painter.setRenderHints(
            QPainter::RenderHint::Antialiasing | QPainter::RenderHint::SmoothPixmapTransform,
            true
        );
        auto pinWidth = PinBodyWidget::WIDTH;
        auto pinHeight = PinBodyWidget::HEIGHT;
        this->setFixedSize(pinWidth, pinHeight);

        auto pinColor = this->getBodyColor();

        painter.setPen(Qt::PenStyle::NoPen);
        painter.setBrush(pinColor);

        painter.drawRect(
            0,
            0,
            pinWidth,
            pinHeight
        );
    }
}
