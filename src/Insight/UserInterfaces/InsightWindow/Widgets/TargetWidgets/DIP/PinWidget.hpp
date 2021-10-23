#pragma once

#include <QWidget>
#include <cstdint>
#include <QVBoxLayout>
#include <QLabel>
#include <QPainter>

#include "../TargetPinWidget.hpp"

#include "PinBodyWidget.hpp"
#include "src/Targets/TargetVariant.hpp"

namespace Bloom::Widgets::InsightTargetWidgets::Dip
{
    enum Position : std::uint8_t
    {
        TOP,
        BOTTOM
    };

    class PinWidget: public TargetPinWidget
    {
        Q_OBJECT

    public:
        static const int MINIMUM_WIDTH = PinBodyWidget::WIDTH;
        static const int WIDTH_SPACING = 6;
        static const int MAXIMUM_LABEL_COUNT = 3;
        static const int LABEL_HEIGHT = 20;
        static const int MAXIMUM_HEIGHT = PinBodyWidget::HEIGHT
            + (PinWidget::LABEL_HEIGHT * PinWidget::MAXIMUM_LABEL_COUNT) + 40;

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

    protected:
        void paintEvent(QPaintEvent* event) override;
        void drawWidget(QPainter& painter);

    private:
        Position position = Position::TOP;
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
    };
}
