#pragma once

#include <QWidget>

#include "src/Targets/TargetVariant.hpp"
#include "src/Targets/TargetPinDescriptor.hpp"

namespace Bloom::InsightTargetWidgets
{
    using Targets::TargetVariant;
    using Targets::TargetPinDescriptor;
    using Targets::TargetPinType;
    using Targets::TargetPinState;

    class TargetPinWidget: public QWidget
    {
    Q_OBJECT
    protected:
        TargetVariant targetVariant;
        TargetPinDescriptor pinDescriptor;
        std::optional<TargetPinState> pinState;
        bool pinStateChanged = false;

    public:
        TargetPinWidget(QWidget* parent, const TargetPinDescriptor& pinDescriptor, const TargetVariant& targetVariant):
        QWidget(parent), targetVariant(targetVariant), pinDescriptor(pinDescriptor) {
            this->setDisabled(false);
        };

        int getPinNumber() const {
            return this->pinDescriptor.number;
        }

        const TargetPinDescriptor& getPinDescriptor() const {
            return this->pinDescriptor;
        }

        std::optional<TargetPinState> getPinState() const {
            return this->pinState;
        }

        virtual void updatePinState(const TargetPinState& pinState) {
            this->pinStateChanged = !this->pinState.has_value()
                || this->pinState->ioState != pinState.ioState
                || this->pinState->ioDirection != pinState.ioDirection;

            this->pinState = pinState;
        }

        void setDisabled(bool disabled) {
            if (pinDescriptor.type != TargetPinType::GND
                && pinDescriptor.type != TargetPinType::VCC
                && pinDescriptor.type != TargetPinType::UNKNOWN
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
