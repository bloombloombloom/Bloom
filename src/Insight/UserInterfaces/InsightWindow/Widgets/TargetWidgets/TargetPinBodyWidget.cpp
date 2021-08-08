#include <QEvent>

#include "TargetPinBodyWidget.hpp"

using namespace Bloom::Widgets::InsightTargetWidgets;
using namespace Bloom::Targets;

bool TargetPinBodyWidget::event(QEvent* event) {
    if (this->isEnabled()
        && this->pinState.has_value()
        && this->pinState->ioDirection == TargetPinState::IoDirection::OUTPUT
    ) {
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

QColor TargetPinBodyWidget::getBodyColor() {
    auto pinColor = this->defaultBodyColor;

    if (this->pinDescriptor.type == TargetPinType::VCC) {
        pinColor = this->vccBodyColor;

    } else if (this->pinDescriptor.type == TargetPinType::GND) {
        pinColor = this->gndBodyColor;

    } else if (this->pinDescriptor.type == TargetPinType::GPIO) {
        if (this->pinState.has_value()
            && this->pinState->ioState.has_value()
            && this->pinState->ioDirection.has_value()
        ) {
            const auto ioDirection = this->pinState->ioDirection.value();

            if (this->pinState->ioState.value() == TargetPinState::IoState::HIGH) {
                pinColor = ioDirection == TargetPinState::IoDirection::OUTPUT ?
                    this->outputHighBodyColor : this->inputHighBodyColor;
            }

            if (ioDirection == TargetPinState::IoDirection::OUTPUT && !this->hoverActive) {
                pinColor.setAlpha(200);
            }
        }
    }

    if (!this->isEnabled()) {
        pinColor.setAlpha(this->disableAlphaLevel);
    }

    return pinColor;
}
