#include "HexViewerWidget.hpp"

#include <QVBoxLayout>
#include <QScrollBar>
#include <QScrollArea>
#include <QPainter>
#include <cmath>

#include "src/Insight/UserInterfaces/InsightWindow/UiLoader.hpp"

#include "src/Helpers/Paths.hpp"
#include "src/Exceptions/Exception.hpp"
#include "src/Logger/Logger.hpp"

using namespace Bloom::Widgets;
using namespace Bloom::Exceptions;

using Bloom::Targets::TargetMemoryDescriptor;

HexViewerWidget::HexViewerWidget(
    const TargetMemoryDescriptor& targetMemoryDescriptor,
    InsightWorker& insightWorker,
    QWidget* parent
): QWidget(parent), targetMemoryDescriptor(targetMemoryDescriptor), insightWorker(insightWorker) {
    this->setObjectName("hex-viewer-widget");
    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    auto widgetUiFile = QFile(
        QString::fromStdString(Paths::compiledResourcesPath()
            + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget"
            + "/UiFiles/HexViewerWidget.ui"
        )
    );

    if (!widgetUiFile.open(QFile::ReadOnly)) {
        throw Exception("Failed to open HexViewerWidget UI file");
    }

    auto uiLoader = UiLoader(this);
    this->container = uiLoader.load(&widgetUiFile, this);
    this->container->setFixedSize(this->size());
    this->container->setContentsMargins(0, 0, 0, 0);

    this->toolBar = this->container->findChild<QWidget*>("tool-bar");
    this->bottomBar = this->container->findChild<QWidget*>("bottom-bar");

    this->refreshButton = this->container->findChild<QToolButton*>("refresh-memory-btn");
    this->highlightStackMemoryButton = this->container->findChild<SvgToolButton*>("highlight-stack-memory-btn");
    this->highlightFocusedMemoryButton = this->container->findChild<SvgToolButton*>("highlight-focused-memory-btn");

    this->toolBar->setContentsMargins(0, 0, 0, 0);
    this->toolBar->layout()->setContentsMargins(5, 0, 5, 1);

    this->bottomBar->setContentsMargins(0, 0, 0, 0);
    this->bottomBar->layout()->setContentsMargins(5, 0, 5, 0);

    this->hoveredAddressLabel = this->bottomBar->findChild<QLabel*>("byte-address-label");

    auto byteItemGraphicsViewContainer = this->container->findChild<QWidget*>("graphics-view-container");
    auto byteItemGraphicsViewLayout = byteItemGraphicsViewContainer->findChild<QVBoxLayout*>(
        "byte-item-container-layout"
    );
    this->byteItemGraphicsView = new ByteItemContainerGraphicsView(
        targetMemoryDescriptor,
        insightWorker,
        this->settings,
        this->hoveredAddressLabel,
        byteItemGraphicsViewContainer
    );
    this->byteItemGraphicsScene = this->byteItemGraphicsView->getScene();
    byteItemGraphicsViewLayout->insertWidget(0, this->byteItemGraphicsView);

    QObject::connect(
        this->highlightStackMemoryButton,
        &QToolButton::clicked,
        this,
        &HexViewerWidget::toggleStackMemoryHighlighting
    );

    QObject::connect(
        this->highlightFocusedMemoryButton,
        &QToolButton::clicked,
        this,
        &HexViewerWidget::toggleFocusedMemoryHighlighting
    );

    QObject::connect(
        &insightWorker,
        &InsightWorker::targetStateUpdated,
        this,
        &HexViewerWidget::onTargetStateChanged
    );

    this->show();
}

void HexViewerWidget::updateValues(const Targets::TargetMemoryBuffer& buffer) {
    this->byteItemGraphicsScene->updateValues(buffer);
}

void HexViewerWidget::setStackPointer(std::uint32_t stackPointer) {
    this->settings.stackPointerAddress = stackPointer;
    this->byteItemGraphicsScene->update();
}

void HexViewerWidget::resizeEvent(QResizeEvent* event) {
    this->container->setFixedSize(
        this->width(),
        this->height()
    );
}

void HexViewerWidget::onTargetStateChanged(Targets::TargetState newState) {
    using Targets::TargetState;
    this->targetState = newState;
}

void HexViewerWidget::onByteWidgetsAdjusted() {
//    const auto& byteWidgetsByRowIndex = this->byteWidgetContainer->byteWidgetsByRowIndex;
//
//    int layoutItemMaxIndex = this->byteWidgetAddressLayout->count() - 1;
//    for (const auto& mappingPair : byteWidgetsByRowIndex) {
//        const auto rowIndex = static_cast<int>(mappingPair.first);
//        const auto& byteWidgets = mappingPair.second;
//
//        if (byteWidgets.empty()) {
//            continue;
//        }
//
//        QLabel* labelWidget;
//        if (rowIndex > layoutItemMaxIndex) {
//            labelWidget = new QLabel(this->byteWidgetAddressContainer);
//            labelWidget->setFixedSize(75, 20);
//            labelWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//
//            this->byteWidgetAddressLayout->addWidget(labelWidget);
//            layoutItemMaxIndex++;
//
//        } else {
//            labelWidget = qobject_cast<QLabel*>(this->byteWidgetAddressLayout->itemAt(rowIndex)->widget());
//        }
//
//        labelWidget->setText(byteWidgets.front()->relativeAddressHex);
//    }
//
//    const auto rowCount = static_cast<int>(byteWidgetsByRowIndex.size());
//    QLayoutItem* labelItem;
//    while ((labelItem = this->byteWidgetAddressLayout->takeAt(rowCount)) != nullptr) {
//        labelItem->widget()->deleteLater();
//    }
}

void HexViewerWidget::toggleStackMemoryHighlighting() {
    auto enable = !this->settings.highlightStackMemory;

    this->highlightStackMemoryButton->setChecked(enable);
    this->settings.highlightStackMemory = enable;

    this->byteItemGraphicsScene->update();
}

void HexViewerWidget::toggleFocusedMemoryHighlighting() {
    auto enable = !this->settings.highlightFocusedMemory;

    this->highlightFocusedMemoryButton->setChecked(enable);
    this->settings.highlightFocusedMemory = enable;

    this->byteItemGraphicsScene->update();
}
