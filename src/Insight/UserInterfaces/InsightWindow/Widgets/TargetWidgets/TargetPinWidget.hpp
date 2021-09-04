#pragma once

#include <QWidget>
#include <utility>

#include "src/Insight/InsightWorker/InsightWorker.hpp"
#include "src/Targets/TargetVariant.hpp"
#include "src/Targets/TargetPinDescriptor.hpp"

namespace Bloom::Widgets::InsightTargetWidgets
{
    class TargetPinWidget: public QWidget
    {
    Q_OBJECT
    protected:
        InsightWorker& insightWorker;

        Targets::TargetVariant targetVariant;
        Targets::TargetPinDescriptor pinDescriptor;
        std::optional<Targets::TargetPinState> pinState;
        bool pinStateChanged = false;

    public:
        TargetPinWidget(
            Targets::TargetPinDescriptor pinDescriptor,
            Targets::TargetVariant targetVariant,
            InsightWorker& insightWorker,
            QWidget* parent
        );

        int getPinNumber() const {
            return this->pinDescriptor.number;
        }

        virtual void updatePinState(const Targets::TargetPinState& pinState) {
            this->pinStateChanged = !this->pinState.has_value()
                || this->pinState->ioState != pinState.ioState
                || this->pinState->ioDirection != pinState.ioDirection;

            this->pinState = pinState;
        }

    public slots:
        virtual void onWidgetBodyClicked();
    };
}
