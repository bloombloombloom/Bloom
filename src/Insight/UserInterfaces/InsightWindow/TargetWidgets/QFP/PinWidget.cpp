#include <QWidget>
#include <QPainter>
#include <QLayout>
#include <cmath>
#include <QEvent>

#include "PinWidget.hpp"
#include "PinBodyWidget.hpp"
#include "src/Logger/Logger.hpp"

using namespace Bloom::InsightTargetWidgets::Qfp;
using namespace Bloom::Targets;

PinWidget::PinWidget(QWidget* parent, const TargetPinDescriptor& pinDescriptor, const TargetVariant& targetVariant):
    TargetPinWidget(parent, pinDescriptor, targetVariant) {
    this->layout = new QVBoxLayout();
    this->layout->setMargin(0);
    this->layout->setSpacing(0);

    auto pinCountPerLayout = (targetVariant.pinDescriptorsByNumber.size() / 4);
    this->isLeftLayout = pinDescriptor.number <= pinCountPerLayout;
    this->isBottomLayout = pinDescriptor.number > pinCountPerLayout && pinDescriptor.number <= (pinCountPerLayout * 2);
    this->isRightLayout = pinDescriptor.number > (pinCountPerLayout * 2) && pinDescriptor.number <= (pinCountPerLayout * 3);
    this->isTopLayout = pinDescriptor.number > (pinCountPerLayout * 3) && pinDescriptor.number <= (pinCountPerLayout * 4);

    this->bodyWidget = new PinBodyWidget(this, this->pinDescriptor, (this->isTopLayout || this->isBottomLayout));

    this->pinDirectionLabel = new QLabel(this);
    this->pinDirectionLabel->setObjectName("target-pin-direction");
    this->pinDirectionLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

    this->pinNameLabel = new QLabel(this);
    this->pinNameLabel->setObjectName("target-pin-name");
    auto pinName = QString::fromStdString(pinDescriptor.name).toUpper();
    pinName.truncate(5);
    this->pinNameLabel->setText(pinName);
    this->pinNameLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

    this->pinNumberLabel = new QLabel(this);
    this->pinNumberLabel->setObjectName("target-pin-number");
    auto pinNumberText = QString::number(pinDescriptor.number);
    pinNumberText.truncate(5);
    this->pinNumberLabel->setText(QString::number(pinDescriptor.number));
    this->pinNumberLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

    if (this->isLeftLayout) {
        this->layout->setAlignment((Qt::AlignmentFlag::AlignVCenter | Qt::AlignmentFlag::AlignRight));
        this->layout->setDirection(QBoxLayout::Direction::RightToLeft);
        this->setFixedSize(PinWidget::MAXIMUM_HORIZONTAL_WIDTH, PinWidget::MAXIMUM_HORIZONTAL_HEIGHT);

    } else if (this->isBottomLayout) {
        this->layout->setAlignment(Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignTop);
        this->layout->setDirection(QBoxLayout::Direction::TopToBottom);
        this->setFixedSize(PinWidget::MAXIMUM_VERTICAL_WIDTH, PinWidget::MAXIMUM_VERTICAL_HEIGHT);

    } else if (this->isRightLayout) {
        this->layout->setAlignment((Qt::AlignmentFlag::AlignVCenter | Qt::AlignmentFlag::AlignLeft));
        this->layout->setDirection(QBoxLayout::Direction::LeftToRight);
        this->setFixedSize(PinWidget::MAXIMUM_HORIZONTAL_WIDTH, PinWidget::MAXIMUM_HORIZONTAL_HEIGHT);

    } else if (this->isTopLayout) {
        this->layout->setAlignment((Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignBottom));
        this->layout->setDirection(QBoxLayout::Direction::BottomToTop);
        this->setFixedSize(PinWidget::MAXIMUM_VERTICAL_WIDTH, PinWidget::MAXIMUM_VERTICAL_HEIGHT);
    }

    this->layout->addWidget(this->bodyWidget);
    this->layout->insertSpacing(1, 3);

    if (this->isLeftLayout || this->isRightLayout) {
        auto stackedLabelLayout = new QVBoxLayout();
        stackedLabelLayout->addWidget(this->pinNameLabel);
        stackedLabelLayout->insertSpacing(3, 2);
        stackedLabelLayout->addWidget(this->pinNumberLabel);
        this->layout->insertSpacing(2, 4);
        this->layout->addLayout(stackedLabelLayout);

        // Adjust the pin name label width for horizontal pins to accommodate for pin names up to 5 characters in length
        this->pinNameLabel->setFixedSize(PinWidget::MAXIMUM_LABEL_WIDTH + 5, PinWidget::MAXIMUM_HORIZONTAL_HEIGHT / 2);
        this->pinNumberLabel->setFixedSize(PinWidget::MAXIMUM_LABEL_WIDTH + 5, PinWidget::MAXIMUM_HORIZONTAL_HEIGHT / 2);

    } else if (this-isTopLayout || this->isBottomLayout) {
        this->layout->addWidget(this->pinNumberLabel);
        this->layout->insertSpacing(3, 2);
        this->layout->addWidget(this->pinNameLabel);

        this->pinNameLabel->setFixedSize(PinBodyWidget::WIDTH, PinWidget::LABEL_HEIGHT - 2);
        this->pinNumberLabel->setFixedSize(PinBodyWidget::WIDTH, PinWidget::LABEL_HEIGHT - 2);
    }

    this->layout->insertSpacing(5, 2);
    this->layout->addWidget(this->pinDirectionLabel);

    this->setLayout(this->layout);

    connect(this->bodyWidget, &PinBodyWidget::clicked, this, &TargetPinWidget::onWidgetBodyClicked);
}

