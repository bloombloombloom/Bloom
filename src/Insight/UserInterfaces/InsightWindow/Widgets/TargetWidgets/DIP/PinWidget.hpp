#pragma once

#include <QWidget>
#include <QLabel>

#include "../TargetPinWidget.hpp"
#include "PinBodyWidget.hpp"
#include "src/Targets/TargetVariant.hpp"

namespace Bloom::Widgets::InsightTargetWidgets::Dip
{
    class PinWidget: public TargetPinWidget
    {
    Q_OBJECT
    private:
        QVBoxLayout* layout = nullptr;
        QLabel* pinNumberLabel = nullptr;
        QLabel* pinNameLabel = nullptr;
        QLabel* pinDirectionLabel = nullptr;
        PinBodyWidget* bodyWidget = nullptr;

        void setLabelColor(const QString& hexColor) {
            auto style = QString("QLabel { color: " + hexColor + "; }");
            if (this->pinNameLabel != nullptr) {
                this->pinNameLabel->setStyleSheet(style);
            }
        }

    public:
        static const int MINIMUM_WIDTH = 30;
        static const int MAXIMUM_LABEL_COUNT = 3;
        static const int LABEL_HEIGHT = 20;
        static const int MAXIMUM_HEIGHT = PinBodyWidget::HEIGHT
            + (PinWidget::LABEL_HEIGHT * PinWidget::MAXIMUM_LABEL_COUNT);

        PinWidget(
            const Targets::TargetPinDescriptor& pinDescriptor,
            const Targets::TargetVariant& targetVariant,
            InsightWorker& insightWorker,
            QWidget* parent
        );

        void updatePinState(const Targets::TargetPinState& pinState) override {
            TargetPinWidget::updatePinState(pinState);

            if (pinState.ioDirection.has_value()) {
                this->pinDirectionLabel->setText(
                    pinState.ioDirection.value() == Targets::TargetPinState::IoDirection::INPUT ? "IN" : "OUT"
                );

            } else {
                this->pinDirectionLabel->setText("");
            }

            if (this->bodyWidget != nullptr) {
                this->bodyWidget->setPinState(pinState);
            }

            this->setLabelColor(this->pinStateChanged ? "#4d7bba" : "#a6a7aa");
        }
    };
}
