#include "PinBodyWidget.hpp"

#include <QPainter>

#include "src/Logger/Logger.hpp"

using namespace Bloom::Widgets::InsightTargetWidgets::Dip;
using namespace Bloom::Targets;

PinBodyWidget::PinBodyWidget(QWidget* parent, Targets::TargetPinDescriptor pinDescriptor)
: TargetPinBodyWidget(parent, std::move(pinDescriptor)) {
    this->setFixedSize(PinBodyWidget::WIDTH, PinBodyWidget::HEIGHT);
}

void PinBodyWidget::paintEvent(QPaintEvent* event) {
    auto painter = QPainter(this);
    this->drawWidget(painter);
}

void PinBodyWidget::drawWidget(QPainter& painter) {
    auto parentWidget = this->parentWidget();

    if (parentWidget == nullptr) {
        Logger::error("PinBodyWidget requires a parent widget");
    }

    painter.setRenderHints(QPainter::RenderHint::Antialiasing | QPainter::RenderHint::SmoothPixmapTransform, true);
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
