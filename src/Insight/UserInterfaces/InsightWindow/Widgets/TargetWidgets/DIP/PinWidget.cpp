#include "PinWidget.hpp"

namespace Bloom::Widgets::InsightTargetWidgets::Dip
{
    using namespace Bloom::Targets;

    PinWidget::PinWidget(
        const TargetPinDescriptor& pinDescriptor,
        const TargetVariant& targetVariant,
        InsightWorker& insightWorker,
        QWidget* parent
    ): TargetPinWidget(pinDescriptor, targetVariant, insightWorker, parent) {
        this->setFixedSize(PinWidget::MINIMUM_WIDTH, PinWidget::MAXIMUM_HEIGHT);

        this->layout = new QVBoxLayout();
        this->layout->setContentsMargins(0, 0, 0, 0);
        this->layout->setSpacing(0);

        this->bodyWidget = new PinBodyWidget(this, this->pinDescriptor);
        this->position = (pinDescriptor.number > (targetVariant.pinDescriptorsByNumber.size() / 2))
            ? Position::TOP : Position::BOTTOM;

        const bool isTopWidget = this->position == Position::TOP;

        this->layout->setAlignment(isTopWidget ? (Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignBottom)
           : (Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignTop));

        this->pinNameLabelText = QString::fromStdString(pinDescriptor.name).toUpper();
        this->pinNameLabelText.truncate(5);

        this->pinNumberLabel = new QLabel(this);
        this->pinNumberLabel->setObjectName("target-pin-number");
        this->pinNumberLabel->setText(QString::number(pinDescriptor.number));
        this->pinNumberLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        if (isTopWidget) {
            this->layout->setDirection(QBoxLayout::Direction::BottomToTop);
        }

        this->layout->addWidget(this->bodyWidget, 0, Qt::AlignmentFlag::AlignHCenter);
        this->layout->insertSpacing(1, PinWidget::PIN_LABEL_SPACING);
        this->layout->addWidget(this->pinNumberLabel, 0, Qt::AlignmentFlag::AlignHCenter);
        this->pinNumberLabel->setFixedHeight(PinWidget::LABEL_HEIGHT);

        this->setLayout(this->layout);

        connect(this->bodyWidget, &PinBodyWidget::clicked, this, &TargetPinWidget::onWidgetBodyClicked);
    }
}
