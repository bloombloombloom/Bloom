#include "PinoutScene.hpp"

#include <cassert>
#include <ranges>
#include <memory>
#include <utility>
#include <QAction>

#include "Qfp/QfpPinout.hpp"
#include "Dip/DipPinout.hpp"

#include "GpioDirectionLabel.hpp"
#include "GpioStateLabel.hpp"
#include "GpioDisabledLabel.hpp"
#include "VerticalLabelGroup.hpp"
#include "HorizontalLabelGroup.hpp"
#include "Pin.hpp"

#include "src/Insight/InsightSignals.hpp"
#include "src/Insight/InsightWorker/InsightWorker.hpp"
#include "src/Insight/InsightWorker/Tasks/ReadTargetGpioPadStates.hpp"
#include "src/Insight/InsightWorker/Tasks/SetTargetGpioPadState.hpp"

namespace Widgets::PinoutWidgets
{
    using Targets::TargetGpioPadState;

    PinoutScene::PinoutScene(const Targets::TargetDescriptor& targetDescriptor, QGraphicsView* parent)
        : QGraphicsScene(parent)
        , parentView(parent)
        , targetDescriptor(targetDescriptor)
    {
        for (const auto& padDescriptor : std::views::values(this->targetDescriptor.padDescriptorsByKey)) {
            if (padDescriptor.type != Targets::TargetPadType::GPIO) {
                continue;
            }

            this->gpioPadDescriptors.emplace_back(&padDescriptor);
            this->gpioPadStatesByPadId.emplace(
                padDescriptor.id,
                TargetGpioPadState{
                    .disabled = true,
                    .value = TargetGpioPadState::State::UNKNOWN,
                    .direction = TargetGpioPadState::DataDirection::UNKNOWN,
                }
            );
        }
    }

    bool PinoutScene::isPinoutSupported(const Targets::TargetPinoutDescriptor& pinoutDescriptor) {
        using Targets::TargetPinoutType;

        const auto pinCount = pinoutDescriptor.pinDescriptors.size();

        if (pinCount > 100) {
            return false;
        }

        if (
            pinoutDescriptor.type != TargetPinoutType::DIP
            && pinoutDescriptor.type != TargetPinoutType::SOIC
            && pinoutDescriptor.type != TargetPinoutType::SSOP
            && pinoutDescriptor.type != TargetPinoutType::QFP
            && pinoutDescriptor.type != TargetPinoutType::QFN
        ) {
            return false;
        }

        if (
            (
                pinoutDescriptor.type == TargetPinoutType::DIP
                || pinoutDescriptor.type == TargetPinoutType::SOIC
                || pinoutDescriptor.type == TargetPinoutType::SSOP
            )
            && pinCount % 2 != 0
        ) {
            return false;
        }

        if (
            (pinoutDescriptor.type == TargetPinoutType::QFP || pinoutDescriptor.type == TargetPinoutType::QFN)
            && (pinCount % 4 != 0 || pinCount <= 4)
        ) {
            return false;
        }

        return true;
    }

    void PinoutScene::setPinout(const Targets::TargetPinoutDescriptor& pinoutDescriptor) {
        using Targets::TargetPinoutType;

        assert(this->isPinoutSupported(pinoutDescriptor));
        this->padLabelsByPadId.clear();

        if (this->pinoutItem != nullptr) {
            this->removeItem(this->pinoutItem);
            delete this->pinoutItem;
            this->pinoutItem = nullptr;
        }

        if (pinoutDescriptor.type == TargetPinoutType::QFP || pinoutDescriptor.type == TargetPinoutType::QFN) {
            auto* pinoutItem = new QfpPinout{pinoutDescriptor, this->targetDescriptor, this->pinoutState};
            this->populatePadLabels(pinoutItem->padDescriptorLabelGroupPairs());
            this->pinoutItem = pinoutItem;
        }

        if (
            pinoutDescriptor.type == TargetPinoutType::DIP
            || pinoutDescriptor.type == TargetPinoutType::SOIC
            || pinoutDescriptor.type == TargetPinoutType::SSOP
        ) {
            auto* pinoutItem = new DipPinout{pinoutDescriptor, this->targetDescriptor, this->pinoutState};
            this->populatePadLabels(pinoutItem->padDescriptorLabelGroupPairs());
            this->pinoutItem = pinoutItem;
        }

        assert(this->pinoutItem != nullptr);
        this->pinoutItem->refreshGeometry();
        this->addItem(this->pinoutItem);

        this->adjustSize();
        this->update();
    }

    void PinoutScene::setDisabled(bool disabled) {
        if (this->pinoutItem == nullptr) {
            return;
        }

        this->pinoutItem->setEnabled(!disabled);
        this->update();
    }

    void PinoutScene::refreshPadStates(const std::optional<std::function<void(void)>>& callback) {
        const auto refreshTask = QSharedPointer<ReadTargetGpioPadStates>{
            new ReadTargetGpioPadStates{this->gpioPadDescriptors},
            &QObject::deleteLater
        };

        QObject::connect(
            refreshTask.get(),
            &ReadTargetGpioPadStates::targetGpioPadStatesRead,
            this,
            &PinoutScene::updatePadStates
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

    void PinoutScene::adjustSize() {
        if (this->pinoutItem == nullptr) {
            return;
        }

        this->setSceneRect(this->pinoutItem->boundingRect());
    }

    void PinoutScene::populatePadLabels(
        const std::vector<
            Pair<const Targets::TargetPadDescriptor&, LabelGroupInterface*>
        >& padDescriptorLabelGroupPairs
    ) {
        for (auto& [padDescriptor, labelGroup] : padDescriptorLabelGroupPairs) {
            auto padLabels = PadLabels{};

            auto gpioPadStateIt = this->gpioPadStatesByPadId.find(padDescriptor.id);
            if (gpioPadStateIt != this->gpioPadStatesByPadId.end()) {
                const auto& gpioPadState = gpioPadStateIt->second;

                if (padDescriptor.type == Targets::TargetPadType::GPIO) {
                    auto gpioDirectionLabel = std::make_unique<GpioDirectionLabel>(gpioPadState);
                    auto gpioStateLabel = std::make_unique<GpioStateLabel>(gpioPadState);
                    auto gpioDisabledLabel = std::make_unique<GpioDisabledLabel>();

                    gpioDirectionLabel->enabled = false;
                    gpioStateLabel->enabled = false;
                    gpioDisabledLabel->enabled = false;

                    padLabels.gpioDirection = gpioDirectionLabel.get();
                    padLabels.gpioState = gpioStateLabel.get();
                    padLabels.gpioDisabled = gpioDisabledLabel.get();

                    labelGroup->insertLabel(std::move(gpioDirectionLabel));
                    labelGroup->insertLabel(std::move(gpioStateLabel));
                    labelGroup->insertLabel(std::move(gpioDisabledLabel));
                }
            }

            this->padLabelsByPadId.emplace(padDescriptor.id, padLabels);
        }
    }

    void PinoutScene::updatePadStates(const Targets::TargetGpioPadDescriptorAndStatePairs& padStatePairs) {
        if (this->pinoutItem == nullptr) {
            return;
        }

        for (const auto& [padDescriptor, padState] : padStatePairs) {
            const auto currentPadStateIt = this->gpioPadStatesByPadId.find(padDescriptor.id);
            if (currentPadStateIt == this->gpioPadStatesByPadId.end()) {
                continue;
            }

            auto& currentPadState = currentPadStateIt->second;

            const auto padLabelsIt = this->padLabelsByPadId.find(padDescriptor.id);
            if (padLabelsIt == this->padLabelsByPadId.end()) {
                continue;
            }

            auto& padLabels = padLabelsIt->second;
            padLabels.disableGpioLabels();

            if (!padState.disabled) {
                if (padState.direction != TargetGpioPadState::DataDirection::UNKNOWN) {
                    padLabels.gpioDirection->enabled = true;
                    padLabels.gpioDirection->changed = padState.direction != currentPadState.direction;
                }

                if (padState.value != TargetGpioPadState::State::UNKNOWN) {
                    padLabels.gpioState->enabled = true;
                    padLabels.gpioState->changed = padState.value != currentPadState.value;
                }
            } else {
                padLabels.gpioDisabled->enabled = true;
            }

            currentPadState = padState;
        }

        this->pinoutItem->refreshGeometry();
        this->adjustSize();
        this->update();
    }

    void PinoutScene::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) {
        if (!this->pinoutItem->isEnabled()) {
            return;
        }

        const auto mousePosition = mouseEvent->scenePos();

        const auto hoveredItems = this->items(mousePosition);
        if (!hoveredItems.empty()) {
            this->update();
            const auto* hoveredItem = hoveredItems.first();

            const auto* hoveredVerticalLabelGroup = dynamic_cast<const VerticalLabelGroup*>(hoveredItem);
            if (hoveredVerticalLabelGroup != nullptr) {
                this->pinoutState.hoveredPinNumber = hoveredVerticalLabelGroup->pinNumber;
                return;
            }

            const auto* hoveredHorizontalLabelGroup = dynamic_cast<const HorizontalLabelGroup*>(hoveredItem);
            if (hoveredHorizontalLabelGroup != nullptr) {
                this->pinoutState.hoveredPinNumber = hoveredHorizontalLabelGroup->pinNumber;
                return;
            }

            const auto* hoveredPin = dynamic_cast<const Pin*>(hoveredItem);
            if (hoveredPin != nullptr) {
                this->pinoutState.hoveredPinNumber = hoveredPin->number;
                return;
            }
        }

        if (this->pinoutState.hoveredPinNumber.has_value()) {
            this->pinoutState.hoveredPinNumber = std::nullopt;
            this->update();
        }
    }

    void PinoutScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) {
        if (mouseEvent->button() != Qt::MouseButton::LeftButton) {
            return QGraphicsScene::mouseReleaseEvent(mouseEvent);
        }

        const auto mousePosition = mouseEvent->scenePos();

        const auto clickedItems = this->items(mousePosition);
        if (clickedItems.empty()) {
            return;
        }

        const auto* clickedPinItem = dynamic_cast<const Pin*>(clickedItems.first());
        if (clickedPinItem == nullptr || !clickedPinItem->padDescriptor.has_value()) {
            return;
        }

        this->toggleOutputPad(clickedPinItem->padDescriptor->get());
    }

    void PinoutScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
        const auto screenPosition = event->screenPos();

        const auto clickedItems = this->items(event->scenePos());
        if (clickedItems.empty()) {
            return;
        }

        const auto* clickedPinItem = dynamic_cast<const Pin*>(clickedItems.first());
        if (clickedPinItem != nullptr) {
            if (!clickedPinItem->padDescriptor.has_value()) {
                return;
            }

            auto* menu = this->contextMenuForPad(clickedPinItem->padDescriptor->get());
            menu->exec(screenPosition);
            return;
        }

        const auto* clickedVerticalLabelGroupItem = dynamic_cast<const VerticalLabelGroup*>(clickedItems.first());
        if (clickedVerticalLabelGroupItem != nullptr) {
            if (!clickedVerticalLabelGroupItem->padDescriptor.has_value()) {
                return;
            }

            auto* menu = this->contextMenuForPad(clickedVerticalLabelGroupItem->padDescriptor->get());
            menu->exec(screenPosition);
            return;
        }

        const auto* clickedHorizontalLabelGroupItem = dynamic_cast<const HorizontalLabelGroup*>(clickedItems.first());
        if (clickedHorizontalLabelGroupItem != nullptr) {
            if (!clickedHorizontalLabelGroupItem->padDescriptor.has_value()) {
                return;
            }

            auto* menu = this->contextMenuForPad(clickedHorizontalLabelGroupItem->padDescriptor->get());
            menu->exec(screenPosition);
            return;
        }
    }

    QMenu* PinoutScene::contextMenuForPad(const Targets::TargetPadDescriptor& padDescriptor) {
        using Targets::TargetGpioPadState;

        auto* menu = new QMenu{this->parentView};
        menu->setAttribute(Qt::WA_DeleteOnClose);

        auto* toggleOutputAction = new QAction{"Toggle output state", menu};
        QObject::connect(
            toggleOutputAction,
            &QAction::triggered,
            this,
            [this, &padDescriptor] () {
                this->toggleOutputPad(padDescriptor);
            }
        );

        const auto currentPadStateIt = this->gpioPadStatesByPadId.find(padDescriptor.id);
        if (
            currentPadStateIt == this->gpioPadStatesByPadId.end()
            || currentPadStateIt->second.direction != TargetGpioPadState::DataDirection::OUTPUT
        ) {
            toggleOutputAction->setDisabled(true);
        }

        menu->addAction(toggleOutputAction);
        return menu;
    }

    void PinoutScene::toggleOutputPad(const Targets::TargetPadDescriptor& padDescriptor) {
        using Targets::TargetGpioPadState;

        const auto currentPadStateIt = this->gpioPadStatesByPadId.find(padDescriptor.id);
        if (currentPadStateIt == this->gpioPadStatesByPadId.end()) {
            return;
        }

        const auto& currentPadState = currentPadStateIt->second;
        if (currentPadState.disabled || currentPadState.direction != TargetGpioPadState::DataDirection::OUTPUT) {
            return;
        }

        auto newPadState = currentPadState;
        newPadState.value = currentPadState.value == TargetGpioPadState::State::HIGH
            ? TargetGpioPadState::State::LOW
            : TargetGpioPadState::State::HIGH;

        const auto setPadStateTask = QSharedPointer<SetTargetGpioPadState>{
            new SetTargetGpioPadState{padDescriptor, newPadState},
            &QObject::deleteLater
        };

        QObject::connect(
            setPadStateTask.get(),
            &InsightWorkerTask::completed,
            this,
            [this, &padDescriptor, newPadState] {
                this->updatePadStates(Targets::TargetGpioPadDescriptorAndStatePairs{{padDescriptor, newPadState}});
            }
        );

        InsightWorker::queueTask(setPadStateTask);
    }
}
