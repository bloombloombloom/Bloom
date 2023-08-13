#include "TargetPackageWidget.hpp"

#include <QEvent>

#include "src/Insight/InsightSignals.hpp"
#include "src/Insight/InsightWorker/InsightWorker.hpp"
#include "src/Insight/InsightWorker/Tasks/RefreshTargetPinStates.hpp"

namespace Widgets::InsightTargetWidgets
{
    using Targets::TargetState;

    TargetPackageWidget::TargetPackageWidget(
        Targets::TargetVariant targetVariant,
        QWidget* parent
    )
        : QWidget(parent)
        , targetVariant(std::move(targetVariant))
    {
        auto* insightSignals = InsightSignals::instance();

        QObject::connect(
            insightSignals,
            &InsightSignals::targetStateUpdated,
            this,
            &TargetPackageWidget::onTargetStateChanged
        );

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

    void TargetPackageWidget::refreshPinStates(std::optional<std::function<void(void)>> callback) {
        const auto refreshTask = QSharedPointer<RefreshTargetPinStates>(
            new RefreshTargetPinStates(this->targetVariant.id),
            &QObject::deleteLater
        );

        QObject::connect(
            refreshTask.get(),
            &RefreshTargetPinStates::targetPinStatesRetrieved,
            this,
            &TargetPackageWidget::updatePinStates
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

    void TargetPackageWidget::updatePinStates(const Targets::TargetPinStateMapping& pinStatesByNumber) {
        for (auto& pinWidget : this->pinWidgets) {
            const auto pinStateIt = pinStatesByNumber.find(pinWidget->getPinNumber());

            if (pinStateIt != pinStatesByNumber.end()) {
                pinWidget->updatePinState(pinStateIt->second);
            }
        }

        this->update();
    }

    void TargetPackageWidget::onTargetStateChanged(TargetState newState) {
        if (this->targetState == newState) {
            return;
        }

        this->targetState = newState;
    }

    void TargetPackageWidget::onProgrammingModeEnabled() {
        if (this->targetState == TargetState::STOPPED) {
            this->setDisabled(true);
        }
    }

    void TargetPackageWidget::onProgrammingModeDisabled() {
        if (this->targetState == TargetState::STOPPED) {
            this->setDisabled(false);
        }
    }
}
