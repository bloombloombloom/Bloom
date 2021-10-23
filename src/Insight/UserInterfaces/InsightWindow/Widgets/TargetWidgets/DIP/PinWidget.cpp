#include "PinWidget.hpp"

using namespace Bloom::Widgets::InsightTargetWidgets::Dip;
using namespace Bloom::Targets;

PinWidget::PinWidget(
    const TargetPinDescriptor& pinDescriptor,
    const TargetVariant& targetVariant,
    InsightWorker& insightWorker,
    QWidget* parent
): TargetPinWidget(pinDescriptor, targetVariant, insightWorker, parent) {
    this->setFixedSize(PinWidget::MINIMUM_WIDTH + PinWidget::WIDTH_SPACING, PinWidget::MAXIMUM_HEIGHT);

    this->layout = new QVBoxLayout();
    this->layout->setContentsMargins(0, 0, 0, 0);
    this->layout->setSpacing(0);

    this->bodyWidget = new PinBodyWidget(this, this->pinDescriptor);
    this->position = (pinDescriptor.number > (targetVariant.pinDescriptorsByNumber.size() / 2))
        ? Position::TOP : Position::BOTTOM;

    const bool isTopWidget = this->position == Position::TOP;

    this->layout->setAlignment(isTopWidget ? (Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignBottom)
       : (Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignTop));

    this->pinDirectionLabel = new QLabel(this);
    this->pinDirectionLabel->setObjectName("target-pin-direction");
    this->pinDirectionLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

    auto pinName = QString::fromStdString(pinDescriptor.name).toUpper();
    this->pinNameLabel = new QLabel(this);
    this->pinNameLabel->setObjectName("target-pin-name");
    this->pinNameLabel->setToolTip(pinName);
    if (pinName.size() > 4) {
        pinName.truncate(4);
    }
    this->pinNameLabel->setText(pinName);
    this->pinNameLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);
    this->pinNameLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    this->pinNumberLabel = new QLabel(this);
    this->pinNumberLabel->setObjectName("target-pin-number");
    this->pinNumberLabel->setText(QString::number(pinDescriptor.number));
    this->pinNumberLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

    if (isTopWidget) {
        this->layout->setDirection(QBoxLayout::Direction::BottomToTop);
    }

    this->layout->addWidget(this->bodyWidget, 0, Qt::AlignmentFlag::AlignHCenter);
    this->layout->insertSpacing(1, 3);
    this->layout->addWidget(this->pinNumberLabel, 0, Qt::AlignmentFlag::AlignHCenter);
    this->layout->insertSpacing(3, (this->pinDescriptor.number % 2 == 0) ? 20 : 2);
    this->layout->addWidget(this->pinNameLabel, 0, Qt::AlignmentFlag::AlignHCenter);
    this->layout->insertSpacing(5, (this->pinDescriptor.number % 2 != 0) ? 20 : 2);
    this->layout->addWidget(this->pinDirectionLabel, 0, Qt::AlignmentFlag::AlignHCenter);
    this->pinNameLabel->setFixedHeight(PinWidget::LABEL_HEIGHT - 2);
    this->pinNumberLabel->setFixedHeight(PinWidget::LABEL_HEIGHT - 2);

    this->setLayout(this->layout);

    connect(this->bodyWidget, &PinBodyWidget::clicked, this, &TargetPinWidget::onWidgetBodyClicked);
}

void PinWidget::paintEvent(QPaintEvent* event) {
    auto painter = QPainter(this);
    this->drawWidget(painter);
}

void PinWidget::drawWidget(QPainter& painter) {
    painter.setPen(QPen(QColor("#4F4F4F"), 1));

    if (this->pinDescriptor.number % 2 == 0) {
        /*
         * Minus 17 for the length of the line
         * Plus/minus 3 to account for spacing between the pin body and number label
         */
        const auto yOffset = this->position == Position::TOP
            ? PinWidget::MAXIMUM_HEIGHT - PinBodyWidget::HEIGHT - PinWidget::LABEL_HEIGHT - 17 - 3
            : PinBodyWidget::HEIGHT + PinWidget::LABEL_HEIGHT + 3;

        painter.drawLine(QLine(
            (PinWidget::MINIMUM_WIDTH + PinWidget::WIDTH_SPACING) / 2,
            yOffset,
            (PinWidget::MINIMUM_WIDTH + PinWidget::WIDTH_SPACING) / 2,
            yOffset + 17
        ));

    } else if (this->pinState.has_value()) {
        // Plus/minus 2 because it looks nicer
        const auto yOffset = this->position == Position::TOP
            ? PinWidget::MAXIMUM_HEIGHT - PinBodyWidget::HEIGHT - (PinWidget::LABEL_HEIGHT * 2) - 17 - 3 + 2
            : PinBodyWidget::HEIGHT + (PinWidget::LABEL_HEIGHT * 2) + 3 - 2;

        painter.drawLine(QLine(
            (PinWidget::MINIMUM_WIDTH + PinWidget::WIDTH_SPACING) / 2,
            yOffset,
            (PinWidget::MINIMUM_WIDTH + PinWidget::WIDTH_SPACING) / 2,
            yOffset + 17
        ));
    }
}
