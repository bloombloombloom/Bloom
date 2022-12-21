#pragma once

#include <QWidget>
#include <utility>
#include <optional>

#include "src/Targets/TargetVariant.hpp"
#include "src/Targets/TargetPinDescriptor.hpp"

namespace Bloom::Widgets::InsightTargetWidgets
{
    class TargetPinWidget: public QWidget
    {
        Q_OBJECT

    public:
        TargetPinWidget(
            Targets::TargetPinDescriptor pinDescriptor,
            Targets::TargetVariant targetVariant,
            QWidget* parent
        );

        int getPinNumber() const {
            return this->pinDescriptor.number;
        }

        const std::optional<Targets::TargetPinState>& getPinState() const {
            return this->pinState;
        }

        virtual void updatePinState(const Targets::TargetPinState& pinState) {
            this->pinStateChanged = !this->pinState.has_value()
                || this->pinState->ioState != pinState.ioState
                || this->pinState->ioDirection != pinState.ioDirection;

            this->pinState = pinState;
        }

        bool hasPinStateChanged() const {
            return this->pinStateChanged;
        }

    public slots:
        virtual void onWidgetBodyClicked();

    protected:
        Targets::TargetVariant targetVariant;
        Targets::TargetPinDescriptor pinDescriptor;
        std::optional<Targets::TargetPinState> pinState;
        bool pinStateChanged = false;
    };
}
