#include "DualInlinePackageWidget.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <vector>
#include <QEvent>
#include <QFile>

#include "src/Helpers/Paths.hpp"

using namespace Bloom::Widgets::InsightTargetWidgets::Dip;
using namespace Bloom::Exceptions;

using Bloom::Targets::TargetVariant;

DualInlinePackageWidget::DualInlinePackageWidget(
    const TargetVariant& targetVariant,
    InsightWorker& insightWorker,
    QWidget* parent
): TargetPackageWidget(targetVariant, insightWorker, parent) {
    auto stylesheetFile = QFile(QString::fromStdString(
            Paths::compiledResourcesPath()
            + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetWidgets/DIP/Stylesheets/DualInlinePackage.qss"
        )
    );
    stylesheetFile.open(QFile::ReadOnly);
    this->setStyleSheet(stylesheetFile.readAll());

    this->pinWidgets.reserve(targetVariant.pinDescriptorsByNumber.size());
    this->layout = new QVBoxLayout();
    this->layout->setSpacing(8);
    this->layout->setContentsMargins(0, 0, 0, 0);

    this->topPinLayout = new QHBoxLayout();
    this->topPinLayout->setSpacing(PinWidget::WIDTH_SPACING);
    this->topPinLayout->setDirection(QBoxLayout::Direction::RightToLeft);
    this->bottomPinLayout = new QHBoxLayout();
    this->bottomPinLayout->setSpacing(PinWidget::WIDTH_SPACING);

    for (const auto& [targetPinNumber, targetPinDescriptor]: targetVariant.pinDescriptorsByNumber) {
        auto pinWidget = new PinWidget(targetPinDescriptor, targetVariant, insightWorker, this);
        this->pinWidgets.push_back(pinWidget);

        if (targetPinNumber <= (targetVariant.pinDescriptorsByNumber.size() / 2)) {
            this->bottomPinLayout->addWidget(pinWidget, 0, Qt::AlignmentFlag::AlignHCenter);
        } else {
            this->topPinLayout->addWidget(pinWidget, 0, Qt::AlignmentFlag::AlignRight);
        }
    }

    this->layout->addLayout(this->topPinLayout);
    this->bodyWidget = new BodyWidget(this);
    this->layout->addWidget(this->bodyWidget);
    this->layout->addLayout(this->bottomPinLayout);
    this->setLayout(this->layout);

    const auto bodyWidgetWidth = ((PinWidget::MINIMUM_WIDTH + PinWidget::WIDTH_SPACING)
        * static_cast<int>(this->pinWidgets.size() / 2)) - PinWidget::WIDTH_SPACING + 46;
    const auto bodyWidgetHeight = 150;

    const auto width = bodyWidgetWidth;
    const auto height = (PinWidget::MAXIMUM_HEIGHT * 2) + bodyWidgetHeight + (8 * 3);

    this->bodyWidget->setGeometry(0, PinWidget::MAXIMUM_HEIGHT + 8, width, bodyWidgetHeight);
    this->topPinLayout->setGeometry(QRect(0, 0, width, PinWidget::MAXIMUM_HEIGHT));
    this->bottomPinLayout->setGeometry(
        QRect(
            0,
            (PinWidget::MAXIMUM_HEIGHT + bodyWidgetHeight + (8 * 2)),
            width,
            PinWidget::MAXIMUM_HEIGHT
        )
    );
    this->topPinLayout->setContentsMargins(23, 0, 23, 0);
    this->bottomPinLayout->setContentsMargins( 23, 0, 23, 0);

    this->setFixedSize(width, height);

    this->setGeometry(
        (parent->width() / 2) - (width / 2),
        (parent->height() / 2) - (height / 2),
        width,
        height
    );
}
