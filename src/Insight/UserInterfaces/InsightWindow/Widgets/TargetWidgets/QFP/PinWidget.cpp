#include <QWidget>
#include <QPainter>
#include <QLayout>
#include <cmath>
#include <QEvent>

#include "PinWidget.hpp"
#include "PinBodyWidget.hpp"

namespace Widgets::InsightTargetWidgets::Qfp
{
    using namespace Targets;

    PinWidget::PinWidget(
        const TargetPinDescriptor& pinDescriptor,
        const TargetVariant& targetVariant,
        QWidget* parent
    ): TargetPinWidget(pinDescriptor, targetVariant, parent) {
        this->layout = new QBoxLayout(QBoxLayout::TopToBottom);
        this->layout->setContentsMargins(0, 0, 0, 0);
        this->layout->setSpacing(0);

        auto pinCountPerLayout = (targetVariant.pinDescriptorsByNumber.size() / 4);

        if (pinDescriptor.number <= pinCountPerLayout) {
            this->position = Position::LEFT;

        } else if (pinDescriptor.number > pinCountPerLayout && pinDescriptor.number <= (pinCountPerLayout * 2)) {
            this->position = Position::BOTTOM;

        } else if (pinDescriptor.number > (pinCountPerLayout * 2) && pinDescriptor.number <= (pinCountPerLayout * 3)) {
            this->position = Position::RIGHT;

        } else if (pinDescriptor.number > (pinCountPerLayout * 3) && pinDescriptor.number <= (pinCountPerLayout * 4)) {
            this->position = Position::TOP;
        }

        this->bodyWidget = new PinBodyWidget(
            this,
            this->pinDescriptor,
            (this->position == Position::TOP || this->position == Position::BOTTOM)
        );

        this->pinNameLabelText = QString::fromStdString(pinDescriptor.name).toUpper();
        this->pinNameLabelText.truncate(5);

        this->pinNumberLabel = new Label(this);
        this->pinNumberLabel->setObjectName("target-pin-number");
        auto pinNumberText = QString::number(pinDescriptor.number);
        pinNumberText.truncate(5);
        this->pinNumberLabel->setText(QString::number(pinDescriptor.number));
        this->pinNumberLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        if (this->position == Position::LEFT) {
            this->layout->setAlignment((Qt::AlignmentFlag::AlignVCenter | Qt::AlignmentFlag::AlignRight));
            this->layout->setDirection(QBoxLayout::Direction::RightToLeft);
            this->setFixedSize(PinWidget::MAXIMUM_HORIZONTAL_WIDTH, PinWidget::MAXIMUM_HORIZONTAL_HEIGHT);

        } else if (this->position == Position::BOTTOM) {
            this->layout->setAlignment(Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignTop);
            this->layout->setDirection(QBoxLayout::Direction::TopToBottom);
            this->setFixedSize(PinWidget::MAXIMUM_VERTICAL_WIDTH, PinWidget::MAXIMUM_VERTICAL_HEIGHT);

        } else if (this->position == Position::RIGHT) {
            this->layout->setAlignment((Qt::AlignmentFlag::AlignVCenter | Qt::AlignmentFlag::AlignLeft));
            this->layout->setDirection(QBoxLayout::Direction::LeftToRight);
            this->setFixedSize(PinWidget::MAXIMUM_HORIZONTAL_WIDTH, PinWidget::MAXIMUM_HORIZONTAL_HEIGHT);

        } else if (this->position == Position::TOP) {
            this->layout->setAlignment((Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignBottom));
            this->layout->setDirection(QBoxLayout::Direction::BottomToTop);
            this->setFixedSize(PinWidget::MAXIMUM_VERTICAL_WIDTH, PinWidget::MAXIMUM_VERTICAL_HEIGHT);
        }

        this->layout->addWidget(this->bodyWidget);
        this->layout->addSpacing(3);
        this->layout->addWidget(this->pinNumberLabel);

        if (this->position == Position::LEFT || this->position == Position::RIGHT) {
            this->pinNumberLabel->setFixedSize(
                PinWidget::MAXIMUM_PIN_NUMBER_LABEL_WIDTH,
                PinWidget::MAXIMUM_HORIZONTAL_HEIGHT
            );

        } else if (this->position == Position::TOP || this->position == Position::BOTTOM) {
            this->pinNumberLabel->setFixedSize(PinBodyWidget::WIDTH, PinWidget::LABEL_HEIGHT - 2);
        }

        this->layout->addStretch(1);
        this->setLayout(this->layout);

        connect(this->bodyWidget, &PinBodyWidget::clicked, this, &TargetPinWidget::onWidgetBodyClicked);
    }

    void PinWidget::updatePinState(const Targets::TargetPinState& pinState) {
        TargetPinWidget::updatePinState(pinState);

        if (this->bodyWidget != nullptr) {
            this->bodyWidget->setPinState(pinState);
        }
    }
}
