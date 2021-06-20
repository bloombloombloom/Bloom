#pragma once

#include <QWidget>
#include <utility>
#include <vector>
#include <map>

#include "TargetPinWidget.hpp"
#include "src/Targets/TargetVariant.hpp"
#include "src/Targets/TargetDescriptor.hpp"

namespace Bloom::InsightTargetWidgets
{
    /**
     * Each custom target package widget should be derived from this class.
     */
    class TargetPackageWidget: public QWidget
    {
    Q_OBJECT
    protected:
        Targets::TargetVariant targetVariant;
        std::vector<TargetPinWidget*> pinWidgets;

    public:
        TargetPackageWidget(Targets::TargetVariant targetVariant, QObject* insightWindowObj, QWidget* parent):
        QWidget(parent), targetVariant(std::move(targetVariant)) {};

        virtual void updatePinStates(std::map<int, Targets::TargetPinState> pinStatesByNumber) {
            for (auto& pinWidget : this->pinWidgets) {
                auto pinNumber = pinWidget->getPinNumber();
                if (pinStatesByNumber.contains(pinNumber)) {
                    pinWidget->updatePinState(pinStatesByNumber.at(pinNumber));
                }
            }
        }
    };
}
