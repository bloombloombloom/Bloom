#pragma once

#include <QWidget>
#include <QLabel>

#include "../TargetPinWidget.hpp"
#include "PinBodyWidget.hpp"
#include "src/Targets/TargetVariant.hpp"

namespace Bloom::InsightTargetWidgets::Qfp
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

        bool isLeftLayout = false;
        bool isBottomLayout = false;
        bool isRightLayout = false;
        bool isTopLayout = false;

        void setLabelColor(QString hexColor) {
            auto style = QString("QLabel { color: " + hexColor + "; }");
            if (this->pinNumberLabel != nullptr) {
//                this->pinNumberLabel->setStyleSheet(style);
            }

            if (this->pinNameLabel != nullptr) {
                this->pinNameLabel->setStyleSheet(style);
            }
        }

    public:
        static const int MAXIMUM_SPACING = 8;
        static const int MAXIMUM_LABEL_COUNT = 3;
        static const int LABEL_HEIGHT = 20;
        static const int MAXIMUM_LABEL_WIDTH = PinBodyWidget::WIDTH;
        static const int MAXIMUM_LABEL_HEIGHT = 20;
        static const int MAXIMUM_HORIZONTAL_WIDTH = PinBodyWidget::HEIGHT
            + (PinWidget::MAXIMUM_LABEL_WIDTH * PinWidget::MAXIMUM_LABEL_COUNT);
        static const int MAXIMUM_HORIZONTAL_HEIGHT = PinBodyWidget::WIDTH;
        static const int MAXIMUM_VERTICAL_HEIGHT = PinBodyWidget::HEIGHT
            + (PinWidget::MAXIMUM_LABEL_HEIGHT * PinWidget::MAXIMUM_LABEL_COUNT);
        static const int MAXIMUM_VERTICAL_WIDTH = PinBodyWidget::WIDTH;

        PinWidget(
            QWidget* parent,
            const Targets::TargetPinDescriptor& pinDescriptor,
            const Targets::TargetVariant& targetVariant
        );

        virtual void updatePinState(const Targets::TargetPinState& pinState) override {
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

            this->setLabelColor(this->pinStateChanged ? "#6FA0FF" : "#a6a7aa");
        }
    };
}
