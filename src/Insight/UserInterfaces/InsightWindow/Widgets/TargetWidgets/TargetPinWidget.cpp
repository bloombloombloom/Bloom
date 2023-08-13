#include "TargetPinWidget.hpp"

#include "src/Insight/InsightWorker/InsightWorker.hpp"
#include "src/Insight/InsightWorker/Tasks/SetTargetPinState.hpp"

namespace Widgets::InsightTargetWidgets
{
    using Targets::TargetVariant;
    using Targets::TargetPinDescriptor;
    using Targets::TargetPinType;
    using Targets::TargetPinState;

    TargetPinWidget::TargetPinWidget(
        Targets::TargetPinDescriptor pinDescriptor,
        Targets::TargetVariant targetVariant,
        QWidget* parent
    )
        : QWidget(parent)
        , targetVariant(std::move(targetVariant))
        , pinDescriptor(std::move(pinDescriptor)
    ) {
        if (this->pinDescriptor.type == TargetPinType::UNKNOWN) {
            this->setDisabled(true);
        }
    }

    void TargetPinWidget::onWidgetBodyClicked() {
        // Currently, we only allow users to toggle the IO state of output pins
        if (this->pinState.has_value() && this->pinState.value().ioDirection == TargetPinState::IoDirection::OUTPUT) {
            this->setDisabled(true);

            auto pinState = this->pinState.value();
            pinState.ioState = (pinState.ioState == TargetPinState::IoState::HIGH)
                ? TargetPinState::IoState::LOW
                : TargetPinState::IoState::HIGH;

            const auto setPinStateTask = QSharedPointer<SetTargetPinState>(
                new SetTargetPinState(this->pinDescriptor, pinState),
                &QObject::deleteLater
            );

            QObject::connect(setPinStateTask.get(), &InsightWorkerTask::completed, this, [this, pinState] {
                this->updatePinState(pinState);
                this->setDisabled(false);
            });

            QObject::connect(setPinStateTask.get(), &InsightWorkerTask::failed, this, [this] {
                this->setDisabled(false);
            });

            InsightWorker::queueTask(setPinStateTask);
        }
    }
}
