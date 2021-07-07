#include <QPainter>

#include "BodyWidget.hpp"
#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

using namespace Bloom::Widgets::InsightTargetWidgets::Dip;
using namespace Bloom::Exceptions;

void BodyWidget::paintEvent(QPaintEvent* event) {
    auto painter = QPainter(this);
    this->drawWidget(painter);
}

void BodyWidget::drawWidget(QPainter& painter) {
    auto parentWidget = this->parentWidget();

    if (parentWidget == nullptr) {
        throw Exception("BodyWidget requires a parent widget");
    }

    painter.setRenderHints(QPainter::RenderHint::Antialiasing | QPainter::RenderHint::SmoothPixmapTransform, true);

    // Draw target body
    auto targetBodyColor = this->getBodyColor();
    auto backgroundColor = QColor("#3C3F41");

    if (!this->isEnabled()) {
        targetBodyColor.setAlpha(this->getDisableAlphaLevel());
    }

    painter.setPen(Qt::PenStyle::NoPen);
    painter.setBrush(targetBodyColor);
    auto parentContainerWidth = parentWidget->width();
    auto targetBodyHeight = 150;
    auto targetBodyWidth = parentContainerWidth;

    this->setFixedSize(targetBodyWidth, targetBodyHeight);
    auto targetBodyPoint = QPoint(
        0,
        0
    );

    painter.drawRect(
        targetBodyPoint.x(),
        targetBodyPoint.y(),
        targetBodyWidth,
        targetBodyHeight
    );

    painter.setPen(Qt::PenStyle::NoPen);
    painter.setBrush(backgroundColor);
    painter.drawEllipse(QRectF(
        targetBodyPoint.x() + 10,
        targetBodyPoint.y() + (targetBodyHeight - 30),
        20,
        20
    ));

    painter.drawEllipse(QRectF(
        targetBodyPoint.x() - 15,
        targetBodyPoint.y() + (targetBodyHeight / 2) - 15,
        30,
        30
    ));
}
