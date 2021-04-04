#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <vector>
#include <QEvent>
#include <QFile>

#include "../../InsightWindow.hpp"
#include "DualInlinePackageWidget.hpp"
#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"
#include "PinWidget.hpp"
#include "BodyWidget.hpp"

using namespace Bloom::InsightTargetWidgets::Dip;
using namespace Bloom::Exceptions;

DualInlinePackageWidget::DualInlinePackageWidget(const TargetVariant& targetVariant, QObject* insightWindowObj, QWidget* parent):
TargetPackageWidget(targetVariant, insightWindowObj, parent) {
    auto stylesheetFile = QFile("/home/nav/Projects/Bloom/src/Insight/UserInterfaces/InsightWindow/TargetWidgets/DIP/Stylesheets/DualInlinePackage.qss");
    stylesheetFile.open(QFile::ReadOnly);
    this->setStyleSheet(stylesheetFile.readAll());

    this->pinWidgets.reserve(targetVariant.pinDescriptorsByNumber.size());
    this->layout = new QVBoxLayout();
    this->layout->setSpacing(8);
    this->layout->setMargin(0);

    this->topPinLayout = new QHBoxLayout();
    this->topPinLayout->setSpacing(8);
    this->topPinLayout->setDirection(QBoxLayout::Direction::RightToLeft);
    this->bottomPinLayout = new QHBoxLayout();
    this->bottomPinLayout->setSpacing(8);

    auto insightWindow = qobject_cast<InsightWindow*>(insightWindowObj);
    assert(insightWindow != nullptr);

    for (const auto& [targetPinNumber, targetPinDescriptor]: targetVariant.pinDescriptorsByNumber) {
        auto pinWidget = new PinWidget(this, targetPinDescriptor, targetVariant);
        this->pinWidgets.push_back(pinWidget);

        if (targetPinNumber <= (targetVariant.pinDescriptorsByNumber.size() / 2)) {
            this->bottomPinLayout->addWidget(pinWidget, 0, Qt::AlignmentFlag::AlignHCenter);
        } else {
            this->topPinLayout->addWidget(pinWidget, 0, Qt::AlignmentFlag::AlignRight);
        }

        connect(pinWidget, &TargetPinWidget::toggleIoState, insightWindow, &InsightWindow::togglePinIoState);
    }

    this->layout->addLayout(this->topPinLayout);
    this->bodyWidget = new BodyWidget(this);
    this->layout->addWidget(this->bodyWidget);
    this->layout->addLayout(this->bottomPinLayout);
    this->setLayout(this->layout);

    auto insightWindowWidget = this->window();
    assert(insightWindowWidget != nullptr);

    insightWindowWidget->setMinimumHeight(500);
    insightWindowWidget->setMinimumWidth(1000);
}

void DualInlinePackageWidget::paintEvent(QPaintEvent* event) {
//    Logger::debug("Drawing main package widget");

    auto painter = QPainter(this);
    this->drawWidget(painter);
}

void DualInlinePackageWidget::resizeEvent(QResizeEvent* event) {
//    Logger::debug("RESIZE EVENT");
}

void DualInlinePackageWidget::drawWidget(QPainter& painter) {
    auto parentWidget = this->parentWidget();

    if (parentWidget == nullptr) {
        throw Exception("DualInlinePackageWidget requires a parent widget");
    }

    auto parentContainerHeight = parentWidget->height();
    auto parentContainerWidth = parentWidget->width();

    auto width = ((PinWidget::MINIMUM_WIDTH + 8) * static_cast<int>(this->pinWidgets.size() / 2)) + 46;

    this->topPinLayout->setGeometry(QRect(0, 0, width, PinWidget::MAXIMUM_HEIGHT));
    auto bodyGeometry = QRect(0, this->topPinLayout->geometry().height() + 8, width, this->bodyWidget->height());
    this->bodyWidget->setGeometry(bodyGeometry);
    this->bottomPinLayout->setGeometry(
        QRect(
            0,
            bodyGeometry.top() + bodyGeometry.height() + 8,
            width,
            PinWidget::MAXIMUM_HEIGHT
        )
    );
    this->topPinLayout->setContentsMargins(
        static_cast<int>(width * 0.04),
        0,
        static_cast<int>(width * 0.04),
        0
    );
    this->bottomPinLayout->setContentsMargins(
        static_cast<int>(width * 0.04),
        0,
        static_cast<int>(width * 0.04),
        0
    );

    auto height = (PinWidget::MAXIMUM_HEIGHT * 2) + bodyGeometry.height() + (8 * 3);
    this->setGeometry(
        (parentContainerWidth / 2) - (width / 2),
        (parentContainerHeight / 2) - (height / 2),
        width,
        height
    );
}

