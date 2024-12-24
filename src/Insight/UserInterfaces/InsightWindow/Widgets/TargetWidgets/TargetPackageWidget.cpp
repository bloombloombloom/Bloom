#include "TargetPackageWidget.hpp"

#include <QEvent>

#include "src/Insight/InsightSignals.hpp"
#include "src/Insight/InsightWorker/InsightWorker.hpp"
#include "src/Insight/InsightWorker/Tasks/ReadTargetGpioPadStates.hpp"

namespace Widgets::InsightTargetWidgets
{
    using Targets::TargetState;
    using Targets::TargetExecutionState;

    TargetPackageWidget::TargetPackageWidget(
        const Targets::TargetVariantDescriptor& variantDescriptor,
        const Targets::TargetPinoutDescriptor& pinoutDescriptor,
        const Targets::TargetState& targetState,
        QWidget* parent
    )
        : QWidget(parent)
        , variantDescriptor(variantDescriptor)
        , pinoutDescriptor(pinoutDescriptor)
        , targetState(targetState)
    {
        auto* insightSignals = InsightSignals::instance();

        QObject::connect(
            insightSignals,
            &InsightSignals::programmingModeEnabled,
            this,
            &TargetPackageWidget::onProgrammingModeEnabled
        );

        QObject::connect(
            insightSignals,
            &InsightSignals::programmingModeDisabled,
            this,
            &TargetPackageWidget::onProgrammingModeDisabled
        );

        this->setDisabled(true);
    }

    void TargetPackageWidget::refreshPadStates(std::optional<std::function<void(void)>> callback) {
        const auto refreshTask = QSharedPointer<ReadTargetGpioPadStates>{
            new ReadTargetGpioPadStates{this->padDescriptors},
            &QObject::deleteLater
        };

        QObject::connect(
            refreshTask.get(),
            &ReadTargetGpioPadStates::targetGpioPadStatesRead,
            this,
            &TargetPackageWidget::updatePadStates
        );

        if (callback.has_value()) {
            QObject::connect(
                refreshTask.get(),
                &InsightWorkerTask::completed,
                this,
                callback.value()
            );
        }

        InsightWorker::queueTask(refreshTask);
    }

    void TargetPackageWidget::updatePadStates(const Targets::TargetGpioPadDescriptorAndStatePairs& padStatePairs) {
        for (const auto& [padDescriptor, padState] : padStatePairs) {
            const auto widgetIt = this->pinWidgetsByPadId.find(padDescriptor.id);
            if (widgetIt == this->pinWidgetsByPadId.end()) {
                continue;
            }

            widgetIt->second->updatePadState(padState);
        }

        this->update();
    }

    void TargetPackageWidget::onProgrammingModeEnabled() {
        if (this->targetState.executionState == TargetExecutionState::STOPPED) {
            this->setDisabled(true);
        }
    }

    void TargetPackageWidget::onProgrammingModeDisabled() {
        if (this->targetState.executionState == TargetExecutionState::STOPPED) {
            this->setDisabled(false);
        }
    }
}
