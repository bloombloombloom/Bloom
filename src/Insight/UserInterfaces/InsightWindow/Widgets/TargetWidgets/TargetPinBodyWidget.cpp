#include <QEvent>

#include "TargetPinBodyWidget.hpp"

namespace Widgets::InsightTargetWidgets
{
    using namespace Targets;

    TargetPinBodyWidget::TargetPinBodyWidget(
        const Targets::TargetPinDescriptor& pinDescriptor,
        std::optional<std::reference_wrapper<const Targets::TargetPadDescriptor>> padDescriptor,
        QWidget* parent
    )
        : QWidget(parent)
        , pinDescriptor(pinDescriptor)
        , padDescriptor(padDescriptor)
    {
        this->setObjectName("target-pin-body");
        this->setToolTip(
            this->padDescriptor.has_value()
                ? QString::fromStdString(this->padDescriptor->get().name).toUpper()
                : "Not connected"
        );
    }

    QColor TargetPinBodyWidget::getBodyColor() {
        using Targets::TargetGpioPadState;

        auto pinColor = this->defaultBodyColor;

        if (this->padDescriptor.has_value()) {
            const auto& padDescriptor = this->padDescriptor->get();

            if (padDescriptor.type == TargetPadType::VCC) {
                pinColor = this->vccBodyColor;

            } else if (padDescriptor.type == TargetPadType::GND) {
                pinColor = this->gndBodyColor;

            } else if (padDescriptor.type == TargetPadType::GPIO) {
                if (this->padState.has_value()) {
                    if (this->padState->value == TargetGpioPadState::State::HIGH) {
                        pinColor = this->padState->direction == TargetGpioPadState::DataDirection::OUTPUT
                            ? this->outputHighBodyColor
                            : this->inputHighBodyColor;
                    }

                    if (
                        (
                            this->padState->direction == TargetGpioPadState::DataDirection::OUTPUT
                            || (
                                this->padState->direction == TargetGpioPadState::DataDirection::INPUT
                                && this->padState->value == TargetGpioPadState::State::LOW
                            )
                        )
                        && !this->hoverActive
                    ) {
                        pinColor.setAlpha(220);
                    }
                }
            }
        }

        if (!this->isEnabled()) {
            pinColor.setAlpha(this->disableAlphaLevel);
        }

        return pinColor;
    }

    bool TargetPinBodyWidget::event(QEvent* event) {
        if (this->padState.has_value() && this->padState->direction == TargetGpioPadState::DataDirection::OUTPUT) {
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
}
