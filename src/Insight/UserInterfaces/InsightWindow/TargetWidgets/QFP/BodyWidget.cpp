#include <QPainter>
#include <cmath>

#include "BodyWidget.hpp"
#include "src/Logger/Logger.hpp"

using namespace Bloom::InsightTargetWidgets::Qfp;

void BodyWidget::paintEvent(QPaintEvent* event) {
    auto painter = QPainter(this);
    this->drawWidget(painter);
}

void BodyWidget::drawWidget(QPainter& painter) {
    painter.setRenderHints(QPainter::RenderHint::Antialiasing | QPainter::RenderHint::SmoothPixmapTransform, true);

    auto targetBodyColor = this->getBodyColor();
    auto backgroundColor = QColor("#3C3F41");

    if (!this->isEnabled()) {
        targetBodyColor.setAlpha(this->getDisableAlphaLevel());
    }

    painter.setPen(Qt::PenStyle::NoPen);
    painter.setBrush(targetBodyColor);

    auto containerGeometry = this->geometry();
    auto targetBodyWidth = containerGeometry.width() - 16;
    auto targetBodyHeight = containerGeometry.height() - 16;

    auto targetBodyPoint = QPoint(
        (containerGeometry.width() / 2) - (targetBodyWidth / 2),
        (containerGeometry.height() / 2) - (targetBodyHeight / 2)
    );

    painter.drawRect(
        targetBodyPoint.x(),
        targetBodyPoint.y(),
        targetBodyWidth,
        targetBodyHeight
    );

    painter.setBrush(backgroundColor);
    painter.drawEllipse(QRectF(
        targetBodyPoint.x() + 10,
        targetBodyPoint.y() + 10,
        15,
        15
    ));
}

