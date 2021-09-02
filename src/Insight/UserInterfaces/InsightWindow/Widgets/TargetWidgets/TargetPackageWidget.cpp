#include "TargetPackageWidget.hpp"

#include <QEvent>

#include "src/Insight/InsightWorker/Tasks/RefreshTargetPinStates.hpp"

using namespace Bloom;
using namespace Bloom::Widgets::InsightTargetWidgets;

using Bloom::Targets::TargetState;

TargetPackageWidget::TargetPackageWidget(
    Targets::TargetVariant targetVariant,
    InsightWorker& insightWorker,
    QWidget* parent
): QWidget(parent), targetVariant(std::move(targetVariant)), insightWorker(insightWorker) {
    this->connect(
        &(this->insightWorker),
        &InsightWorker::targetStateUpdated,
        this,
        &TargetPackageWidget::onTargetStateChanged
    );

    this->connect(
        &(this->insightWorker),
        &InsightWorker::targetPinStatesUpdated,
        this,
        [this] (int variantId, const Targets::TargetPinStateMappingType& pinStatesByNumber) {
            if (variantId == this->targetVariant.id) {
                this->updatePinStates(pinStatesByNumber);

                if (this->targetState == TargetState::STOPPED && !this->isEnabled()) {
                    this->setDisabled(false);
                }
            }
        }
    );

    this->setDisabled(true);
}

void TargetPackageWidget::refreshPinStates(std::optional<std::function<void(void)>> callback) {
    auto refreshTask = new RefreshTargetPinStates(this->targetVariant.id);
    this->connect(
        refreshTask,
        &RefreshTargetPinStates::targetPinStatesRetrieved,
        this,
        &TargetPackageWidget::updatePinStates
    );

    if (callback.has_value()) {
        this->connect(
            refreshTask,
            &InsightWorkerTask::completed,
            this,
            callback.value()
        );
    }

    this->insightWorker.queueTask(refreshTask);
}

void TargetPackageWidget::updatePinStates(const Targets::TargetPinStateMappingType& pinStatesByNumber) {
    for (auto& pinWidget : this->pinWidgets) {
        auto pinNumber = pinWidget->getPinNumber();
        if (pinStatesByNumber.contains(pinNumber)) {
            pinWidget->updatePinState(pinStatesByNumber.at(pinNumber));
        }
    }

    this->repaint();
}

void TargetPackageWidget::onTargetStateChanged(TargetState newState) {
    this->targetState = newState;

    if (newState == TargetState::RUNNING) {
        this->setDisabled(true);

    } else if (newState == TargetState::STOPPED) {
        this->refreshPinStates([this] {
            if (this->targetState == TargetState::STOPPED) {
                this->setDisabled(false);
            }
        });
    }
}
