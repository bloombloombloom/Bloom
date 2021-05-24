#pragma once

#include <QWidget>

#include "src/Targets/TargetVariant.hpp"
#include "src/Targets/TargetPinDescriptor.hpp"

namespace Bloom::InsightTargetWidgets
{
    class TargetPinWidget: public QWidget
    {
    Q_OBJECT
    protected:
        Targets::TargetVariant targetVariant;
        Targets::TargetPinDescriptor pinDescriptor;
        std::optional<Targets::TargetPinState> pinState;
        bool pinStateChanged = false;

    public:
        TargetPinWidget(
            QWidget* parent,
            const Targets::TargetPinDescriptor& pinDescriptor,
            const Targets::TargetVariant& targetVariant
        ): QWidget(parent), targetVariant(targetVariant), pinDescriptor(pinDescriptor) {
            this->setDisabled(false);
        };

        int getPinNumber() const {
            return this->pinDescriptor.number;
        }

        const Targets::TargetPinDescriptor& getPinDescriptor() const {
            return this->pinDescriptor;
        }

        std::optional<Targets::TargetPinState> getPinState() const {
            return this->pinState;
        }

        virtual void updatePinState(const Targets::TargetPinState& pinState) {
            this->pinStateChanged = !this->pinState.has_value()
                || this->pinState->ioState != pinState.ioState
                || this->pinState->ioDirection != pinState.ioDirection;

            this->pinState = pinState;
        }

        void setDisabled(bool disabled) {
            if (pinDescriptor.type != Targets::TargetPinType::GND
                && pinDescriptor.type != Targets::TargetPinType::VCC
                && pinDescriptor.type != Targets::TargetPinType::UNKNOWN
            ) {
                QWidget::setDisabled(disabled);

            } else {
                QWidget::setDisabled(true);
            }
        }

    public slots:
        void onWidgetBodyClicked() {
            emit this->toggleIoState(this);
        }

    signals:
        void toggleIoState(TargetPinWidget* pinWidget);
    };
}
