#include <QPainter>
#include <QLayout>
#include <QEvent>
#include <QMenu>
#include <QContextMenuEvent>

#include "PinBodyWidget.hpp"
#include "src/Logger/Logger.hpp"

using namespace Bloom::InsightTargetWidgets::Dip;
using namespace Bloom::Targets;

void PinBodyWidget::paintEvent(QPaintEvent* event) {
    auto painter = QPainter(this);
    this->drawWidget(painter);
}

bool PinBodyWidget::event(QEvent* event) {
    if (this->isEnabled() && this->pinState.has_value() && this->pinState->ioDirection == TargetPinState::IoDirection::OUTPUT) {
        switch (event->type()) {
            case QEvent::Enter: {
                this->hoverActive = true;
                this->repaint();
                break;
            }
            case QEvent::Leave: {
                this->hoverActive = false;
                this->repaint();
                break;
            }
            default: {
                break;
            }
        }
    }

    return QWidget::event(event);
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

    if (this->pinDescriptor.type == TargetPinType::VCC) {
        pinColor = QColor("#ff3d43");

    } else if (this->pinDescriptor.type == TargetPinType::GND) {
        pinColor = QColor("#575757");
    }

    if (this->pinState.has_value()) {
        if (this->pinState->ioState.has_value()
            && this->pinState->ioDirection.has_value()
            && this->pinState->ioState.value() == TargetPinState::IoState::HIGH
        ) {
            pinColor = this->pinState->ioDirection.value() == TargetPinState::IoDirection::OUTPUT ?
                QColor("#3D7F96") : QColor("#A47E3E");
        }
    }

    if (!this->hoverActive) {
        pinColor.setAlpha(225);
    }

    if (!this->isEnabled()) {
        pinColor.setAlpha(this->getDisableAlphaLevel());
    }

    painter.setPen(Qt::PenStyle::NoPen);
    painter.setBrush(pinColor);

    painter.drawRect(
        0,
        0,
        pinWidth,
        pinHeight
    );
}
