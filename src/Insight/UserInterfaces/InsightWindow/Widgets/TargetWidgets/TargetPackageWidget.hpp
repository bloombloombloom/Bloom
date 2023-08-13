#pragma once

#include <QWidget>
#include <utility>
#include <vector>
#include <map>
#include <optional>

#include "TargetPinWidget.hpp"
#include "src/Targets/TargetVariant.hpp"
#include "src/Targets/TargetState.hpp"
#include "src/Targets/TargetRegister.hpp"

namespace Widgets::InsightTargetWidgets
{
    /**
     * Each custom target package widget should be derived from this class.
     */
    class TargetPackageWidget: public QWidget
    {
        Q_OBJECT

    public:
        TargetPackageWidget(Targets::TargetVariant targetVariant, QWidget* parent);
        virtual void refreshPinStates(std::optional<std::function<void(void)>> callback = std::nullopt);

        virtual void setTargetState(Targets::TargetState targetState) {
            this->targetState = targetState;
        }

        QSize sizeHint() const override {
            return this->minimumSize();
        }

        QSize minimumSizeHint() const override {
            return this->sizeHint();
        }

    protected:
        Targets::TargetVariant targetVariant;
        std::vector<TargetPinWidget*> pinWidgets;

        Targets::TargetState targetState = Targets::TargetState::UNKNOWN;

        virtual void updatePinStates(const Targets::TargetPinStateMapping& pinStatesByNumber);
        void onTargetStateChanged(Targets::TargetState newState);
        void onProgrammingModeEnabled();
        void onProgrammingModeDisabled();
    };
}
