#include "TargetPinWidget.hpp"

#include "src/Insight/InsightWorker/Tasks/SetTargetPinState.hpp"

using namespace Bloom;
using namespace Bloom::Widgets::InsightTargetWidgets;

using Bloom::Targets::TargetVariant;
using Bloom::Targets::TargetPinDescriptor;
using Bloom::Targets::TargetPinType;
using Bloom::Targets::TargetPinState;

TargetPinWidget::TargetPinWidget(
    Targets::TargetPinDescriptor pinDescriptor,
    Targets::TargetVariant targetVariant,
    InsightWorker& insightWorker,
    QWidget* parent
): QWidget(parent),
insightWorker(insightWorker),
targetVariant(std::move(targetVariant)),
pinDescriptor(std::move(pinDescriptor)) {
    if (this->pinDescriptor.type == TargetPinType::UNKNOWN) {
        this->setDisabled(true);
    }
}

void TargetPinWidget::onWidgetBodyClicked() {
    // Currently, we only allow users to toggle the IO state of output pins
    if (this->pinState.has_value()
        && this->pinState.value().ioDirection == TargetPinState::IoDirection::OUTPUT
    ) {
        this->setDisabled(true);

        auto pinState = this->pinState.value();
        pinState.ioState = (pinState.ioState == TargetPinState::IoState::HIGH) ?
            TargetPinState::IoState::LOW : TargetPinState::IoState::HIGH;

        auto* setPinStateTask = new SetTargetPinState(this->pinDescriptor, pinState);
        QObject::connect(setPinStateTask, &InsightWorkerTask::completed, this, [this, pinState] {
            this->updatePinState(pinState);
            this->setDisabled(false);
        });

        QObject::connect(setPinStateTask, &InsightWorkerTask::failed, this, [this] {
            this->setDisabled(false);
        });

        this->insightWorker.queueTask(setPinStateTask);
    }
}
