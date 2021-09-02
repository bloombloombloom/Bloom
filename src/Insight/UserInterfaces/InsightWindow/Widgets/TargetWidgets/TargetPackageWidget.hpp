#pragma once

#include <QWidget>
#include <utility>
#include <vector>
#include <map>

#include "src/Insight/InsightWorker/InsightWorker.hpp"

#include "TargetPinWidget.hpp"
#include "src/Targets/TargetVariant.hpp"
#include "src/Targets/TargetDescriptor.hpp"

namespace Bloom::Widgets::InsightTargetWidgets
{
    /**
     * Each custom target package widget should be derived from this class.
     */
    class TargetPackageWidget: public QWidget
    {
    Q_OBJECT
    protected:
        Targets::TargetVariant targetVariant;
        InsightWorker& insightWorker;
        std::vector<TargetPinWidget*> pinWidgets;

        Targets::TargetState targetState = Targets::TargetState::UNKNOWN;

    protected slots:
        virtual void updatePinStates(const Targets::TargetPinStateMappingType& pinStatesByNumber);
        void onTargetStateChanged(Targets::TargetState newState);

    public:
        TargetPackageWidget(Targets::TargetVariant targetVariant, InsightWorker& insightWorker, QWidget* parent);
        virtual void refreshPinStates(std::optional<std::function<void(void)>> callback = std::nullopt);
    };
}
