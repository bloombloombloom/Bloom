#include "TargetPinWidget.hpp"

#include "src/Insight/InsightWorker/InsightWorker.hpp"
#include "src/Insight/InsightWorker/Tasks/SetTargetGpioPadState.hpp"

namespace Widgets::InsightTargetWidgets
{
    TargetPinWidget::TargetPinWidget(
        const Targets::TargetPinDescriptor& pinDescriptor,
        std::optional<std::reference_wrapper<const Targets::TargetPadDescriptor>> padDescriptor,
        const Targets::TargetPinoutDescriptor& pinoutDescriptor,
        QWidget* parent
    )
        : QWidget(parent)
        , pinDescriptor(pinDescriptor)
        , padDescriptor(padDescriptor)
        , pinoutDescriptor(pinoutDescriptor)
    {
        if (!this->padDescriptor.has_value() || this->padDescriptor->get().type == Targets::TargetPadType::OTHER) {
            this->setDisabled(true);
        }
    }

    void TargetPinWidget::onWidgetBodyClicked() {
        using Targets::TargetGpioPadState;

        if (
            this->padDescriptor.has_value()
            && this->padState.has_value()
            && this->padState->direction == TargetGpioPadState::DataDirection::OUTPUT
        ) {
            this->setDisabled(true);

            auto newPadState = *this->padState;
            newPadState.value = this->padState->value == TargetGpioPadState::State::HIGH
                ? TargetGpioPadState::State::LOW
                : TargetGpioPadState::State::HIGH;

            const auto setPadStateTask = QSharedPointer<SetTargetGpioPadState>{
                new SetTargetGpioPadState{this->padDescriptor->get(), newPadState},
                &QObject::deleteLater
            };

            QObject::connect(setPadStateTask.get(), &InsightWorkerTask::completed, this, [this, newPadState] {
                this->updatePadState(newPadState);
                this->setDisabled(false);
            });

            QObject::connect(setPadStateTask.get(), &InsightWorkerTask::failed, this, [this] {
                this->setDisabled(false);
            });

            InsightWorker::queueTask(setPadStateTask);
        }
    }
}
