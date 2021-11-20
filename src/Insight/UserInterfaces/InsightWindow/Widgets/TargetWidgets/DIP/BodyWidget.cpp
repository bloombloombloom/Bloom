#include "BodyWidget.hpp"

using namespace Bloom::Widgets::InsightTargetWidgets::Dip;

BodyWidget::BodyWidget(QWidget* parent): QWidget(parent) {
    this->setObjectName("target-body");

    this->setFixedHeight(BodyWidget::HEIGHT);
}

void BodyWidget::paintEvent(QPaintEvent* event) {
    auto painter = QPainter(this);
    this->drawWidget(painter);
}

void BodyWidget::drawWidget(QPainter& painter) {
    painter.setRenderHints(QPainter::RenderHint::Antialiasing | QPainter::RenderHint::SmoothPixmapTransform, true);

    // Draw target body
    auto targetBodyColor = this->getBodyColor();
    auto backgroundColor = QColor(0x37, 0x38, 0x35);

    if (!this->isEnabled()) {
        targetBodyColor.setAlpha(this->getDisableAlphaLevel());
    }

    painter.setPen(Qt::PenStyle::NoPen);
    painter.setBrush(targetBodyColor);

    auto targetBodyPoint = QPoint(
        0,
        0
    );

    painter.drawRect(
        targetBodyPoint.x(),
        targetBodyPoint.y(),
        this->width(),
        BodyWidget::HEIGHT
    );

    painter.setPen(Qt::PenStyle::NoPen);
    painter.setBrush(backgroundColor);
    painter.drawEllipse(QRectF(
        targetBodyPoint.x() + 10,
        targetBodyPoint.y() + (BodyWidget::HEIGHT - 30),
        18,
        18
    ));

    painter.drawEllipse(QRectF(
        targetBodyPoint.x() - 15,
        targetBodyPoint.y() + (BodyWidget::HEIGHT / 2) - 15,
        24,
        24
    ));
}
