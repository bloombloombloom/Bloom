#include "QuadFlatPackageWidget.hpp"

#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <vector>
#include <QEvent>
#include <QFile>

#include "src/Helpers/Paths.hpp"
#include "PinWidget.hpp"
#include "BodyWidget.hpp"

using namespace Bloom::Widgets::InsightTargetWidgets::Qfp;
using namespace Bloom::Targets;

QuadFlatPackageWidget::QuadFlatPackageWidget(
    const TargetVariant& targetVariant,
    InsightWorker& insightWorker,
    QWidget* parent
): TargetPackageWidget(targetVariant, insightWorker, parent) {
    assert((targetVariant.pinDescriptorsByNumber.size() % 4) == 0);

    auto stylesheetFile = QFile(QString::fromStdString(
            Paths::compiledResourcesPath()
            + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetWidgets//QFP/Stylesheets/QuadFlatPackage.qss"
        )
    );
    stylesheetFile.open(QFile::ReadOnly);
    this->setStyleSheet(stylesheetFile.readAll());

    this->pinWidgets.reserve(targetVariant.pinDescriptorsByNumber.size());
    this->layout = new QVBoxLayout();
    this->layout->setSpacing(0);
    this->layout->setContentsMargins(0, 0, 0, 0);

    this->horizontalLayout = new QHBoxLayout();
    this->horizontalLayout->setSpacing(0);
    this->horizontalLayout->setDirection(QBoxLayout::Direction::LeftToRight);
    this->horizontalLayout->setAlignment(Qt::AlignmentFlag::AlignHCenter);

    this->topPinLayout = new QHBoxLayout();
    this->topPinLayout->setSpacing(PinWidget::WIDTH_SPACING);
    this->topPinLayout->setDirection(QBoxLayout::Direction::RightToLeft);
    this->topPinLayout->setAlignment(Qt::AlignmentFlag::AlignHCenter);

    this->rightPinLayout = new QVBoxLayout();
    this->rightPinLayout->setSpacing(PinWidget::WIDTH_SPACING);
    this->rightPinLayout->setDirection(QBoxLayout::Direction::BottomToTop);
    this->rightPinLayout->setAlignment(Qt::AlignmentFlag::AlignVCenter);

    this->bottomPinLayout = new QHBoxLayout();
    this->bottomPinLayout->setSpacing(PinWidget::WIDTH_SPACING);
    this->bottomPinLayout->setDirection(QBoxLayout::Direction::LeftToRight);
    this->bottomPinLayout->setAlignment(Qt::AlignmentFlag::AlignHCenter);

    this->leftPinLayout = new QVBoxLayout();
    this->leftPinLayout->setSpacing(PinWidget::WIDTH_SPACING);
    this->leftPinLayout->setDirection(QBoxLayout::Direction::TopToBottom);
    this->leftPinLayout->setAlignment(Qt::AlignmentFlag::AlignVCenter);

    const auto pinCountPerLayout = static_cast<int>(targetVariant.pinDescriptorsByNumber.size() / 4);
    for (const auto& [targetPinNumber, targetPinDescriptor]: targetVariant.pinDescriptorsByNumber) {
        auto* pinWidget = new PinWidget(targetPinDescriptor, targetVariant, insightWorker, this);
        this->pinWidgets.push_back(pinWidget);
        TargetPackageWidget::pinWidgets.push_back(pinWidget);

        if (targetPinNumber <= pinCountPerLayout) {
            this->leftPinLayout->addWidget(pinWidget, 0, Qt::AlignmentFlag::AlignRight);

        } else if (targetPinNumber > pinCountPerLayout && targetPinNumber <= (pinCountPerLayout * 2)) {
            this->bottomPinLayout->addWidget(pinWidget, 0, Qt::AlignmentFlag::AlignTop);

        } else if (targetPinNumber > (pinCountPerLayout * 2) && targetPinNumber <= (pinCountPerLayout * 3)) {
            this->rightPinLayout->addWidget(pinWidget, 0, Qt::AlignmentFlag::AlignLeft);

        } else if (targetPinNumber > (pinCountPerLayout * 3) && targetPinNumber <= (pinCountPerLayout * 4)) {
            this->topPinLayout->addWidget(pinWidget, 0, Qt::AlignmentFlag::AlignBottom);
        }
    }

    this->bodyWidget = new BodyWidget(this);
    this->layout->addLayout(this->topPinLayout);
    this->horizontalLayout->addLayout(this->leftPinLayout);
    this->horizontalLayout->addWidget(this->bodyWidget);
    this->horizontalLayout->addLayout(this->rightPinLayout);
    this->layout->addLayout(this->horizontalLayout);
    this->layout->addLayout(this->bottomPinLayout);
    this->setLayout(this->layout);

    // Layout sizing, positioning and padding
    const auto verticalPinWidgetHeight = PinWidget::MAXIMUM_VERTICAL_HEIGHT;
    const auto verticalPinWidgetWidth = PinWidget::MAXIMUM_VERTICAL_WIDTH;
    const auto horizontalPinWidgetHeight = PinWidget::MAXIMUM_HORIZONTAL_HEIGHT;
    const auto horizontalPinWidgetWidth = PinWidget::MAXIMUM_HORIZONTAL_WIDTH;

    /*
     * Horizontal layouts are the right and left pin layouts - the ones that hold horizontal pin widgets.
     * The bottom and top layouts are vertical layouts, as they hold the vertical pin widgets.
     */
    const auto horizontalLayoutHeight = ((horizontalPinWidgetHeight + PinWidget::WIDTH_SPACING) * pinCountPerLayout
        + PinWidget::PIN_WIDGET_LAYOUT_PADDING - PinWidget::WIDTH_SPACING);

    const auto verticalLayoutWidth = ((verticalPinWidgetWidth + PinWidget::WIDTH_SPACING) * pinCountPerLayout
        + PinWidget::PIN_WIDGET_LAYOUT_PADDING - PinWidget::WIDTH_SPACING);

    /*
     * + 16 for the spacing between the package body and the pins (8 pixels on each side)
     *
     * Also, the width is a little smaller than the height because of the layout of the horizontal pin labels, but
     * we just use the same value as the height here (to contain and center the widget), as it looks nicer.
     */
    const auto height = horizontalLayoutHeight + (verticalPinWidgetHeight * 2);
    const auto width = height;

    this->topPinLayout->insertSpacing(0, horizontalPinWidgetWidth);
    this->topPinLayout->addSpacing(horizontalPinWidgetWidth);

    this->bottomPinLayout->insertSpacing(0, horizontalPinWidgetWidth);
    this->bottomPinLayout->addSpacing(horizontalPinWidgetWidth);

    this->leftPinLayout->setGeometry(QRect(
        0,
        verticalPinWidgetHeight,
        horizontalPinWidgetWidth,
        horizontalLayoutHeight
    ));

    this->bodyWidget->setFixedSize(verticalLayoutWidth, horizontalLayoutHeight);

    this->rightPinLayout->setGeometry(QRect(
        horizontalLayoutHeight + horizontalPinWidgetWidth,
        verticalPinWidgetHeight,
        horizontalPinWidgetWidth,
        horizontalLayoutHeight
    ));

    const auto pinWidgetLayoutMargin = PinWidget::PIN_WIDGET_LAYOUT_PADDING / 2;

    this->topPinLayout->setContentsMargins(
        pinWidgetLayoutMargin,
        0,
        pinWidgetLayoutMargin,
        0
    );

    this->bottomPinLayout->setContentsMargins(
        pinWidgetLayoutMargin,
        0,
        pinWidgetLayoutMargin,
        0
    );

    this->leftPinLayout->setContentsMargins(
        0,
        pinWidgetLayoutMargin,
        0,
        pinWidgetLayoutMargin
    );

    this->rightPinLayout->setContentsMargins(
        0,
        pinWidgetLayoutMargin,
        0,
        pinWidgetLayoutMargin
    );

    this->setFixedSize(width, height);

    // Set the fixed size and center the widget
    this->setGeometry(
        (parent->width() / 2) - (width / 2),
        (parent->height() / 2) - (height / 2),
        width,
        height
    );
}
