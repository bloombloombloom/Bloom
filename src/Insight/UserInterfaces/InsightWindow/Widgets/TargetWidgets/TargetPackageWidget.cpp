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
    QObject::connect(
        &(this->insightWorker),
        &InsightWorker::targetStateUpdated,
        this,
        &TargetPackageWidget::onTargetStateChanged
    );

    QObject::connect(
        &(this->insightWorker),
        &InsightWorker::targetRegistersWritten,
        this,
        &TargetPackageWidget::onRegistersWritten
    );

    this->setDisabled(true);
}

void TargetPackageWidget::refreshPinStates(std::optional<std::function<void(void)>> callback) {
    auto* refreshTask = new RefreshTargetPinStates(this->targetVariant.id);
    QObject::connect(
        refreshTask,
        &RefreshTargetPinStates::targetPinStatesRetrieved,
        this,
        &TargetPackageWidget::updatePinStates
    );

    if (callback.has_value()) {
        QObject::connect(
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

    this->update();
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

void TargetPackageWidget::onRegistersWritten(Targets::TargetRegisters targetRegisters) {
    if (this->targetState != TargetState::STOPPED) {
        return;
    }

    // If a PORT register was just updated, refresh pin states.
    for (const auto& targetRegister : targetRegisters) {
        if (targetRegister.descriptor.type == Targets::TargetRegisterType::PORT_REGISTER) {
            this->refreshPinStates();
            return;
        }
    }
}
