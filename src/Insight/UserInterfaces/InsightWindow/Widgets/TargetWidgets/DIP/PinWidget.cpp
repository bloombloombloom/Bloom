#include <QWidget>
#include <QPainter>
#include <QLayout>
#include <cmath>
#include <QEvent>

#include "PinWidget.hpp"
#include "PinBodyWidget.hpp"
#include "src/Logger/Logger.hpp"

using namespace Bloom::Widgets::InsightTargetWidgets::Dip;
using namespace Bloom::Targets;

PinWidget::PinWidget(QWidget* parent, const TargetPinDescriptor& pinDescriptor, const TargetVariant& targetVariant):
    TargetPinWidget(parent, pinDescriptor, targetVariant) {
    this->layout = new QVBoxLayout();
    this->layout->setMargin(0);
    this->layout->setSpacing(0);

    this->bodyWidget = new PinBodyWidget(this, this->pinDescriptor);
    bool isTopWidget = pinDescriptor.number > (targetVariant.pinDescriptorsByNumber.size() / 2);

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

    this->pinNumberLabel = new QLabel(this);
    this->pinNumberLabel->setObjectName("target-pin-number");
    this->pinNumberLabel->setText(QString::number(pinDescriptor.number));
    this->pinNumberLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

    if (isTopWidget) {
        this->layout->setDirection(QBoxLayout::Direction::BottomToTop);
    }

    this->layout->addWidget(this->bodyWidget);
    this->layout->insertSpacing(1, 3);
    this->layout->addWidget(this->pinNumberLabel);
    this->layout->insertSpacing(3, 2);
    this->layout->addWidget(this->pinNameLabel);
    this->layout->insertSpacing(5, 2);
    this->layout->addWidget(this->pinDirectionLabel);
    this->pinNameLabel->setFixedSize(PinBodyWidget::WIDTH, PinWidget::LABEL_HEIGHT - 2);
    this->pinNumberLabel->setFixedSize(PinBodyWidget::WIDTH, PinWidget::LABEL_HEIGHT - 2);

    this->setFixedSize(PinWidget::MINIMUM_WIDTH, PinWidget::MAXIMUM_HEIGHT);
    this->setLayout(this->layout);

    connect(this->bodyWidget, &PinBodyWidget::clicked, this, &TargetPinWidget::onWidgetBodyClicked);
}
