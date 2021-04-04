#pragma once

#include <QWidget>
#include <vector>
#include <map>

#include "TargetPinWidget.hpp"
#include "src/Targets/TargetVariant.hpp"
#include "src/Targets/TargetDescriptor.hpp"

namespace Bloom::InsightTargetWidgets
{
    using Targets::TargetVariant;
    using Targets::TargetPinState;

    /**
     * Each custom target package widget should be derived from this class.
     */
    class TargetPackageWidget: public QWidget
    {
    Q_OBJECT
    protected:
        TargetVariant targetVariant;
        std::vector<TargetPinWidget*> pinWidgets;

    public:
        TargetPackageWidget(const TargetVariant& targetVariant, QObject* insightWindowObj, QWidget* parent):
        QWidget(parent), targetVariant(targetVariant) {};

        virtual void updatePinStates(std::map<int, TargetPinState> pinStatesByNumber) {
            for (auto& pinWidget : this->pinWidgets) {
                auto pinNumber = pinWidget->getPinNumber();
                if (pinStatesByNumber.contains(pinNumber)) {
                    pinWidget->updatePinState(pinStatesByNumber.at(pinNumber));
                }
            }
        }
    };
}
