#include "HexViewerWidget.hpp"

#include <QVBoxLayout>
#include <QTableWidget>
#include <QScrollBar>
#include <QScrollArea>
#include <QPainter>
#include <cmath>

#include "src/Insight/UserInterfaces/InsightWindow/UiLoader.hpp"

#include "src/Helpers/Paths.hpp"
#include "src/Exceptions/Exception.hpp"

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

    this->toolBar->setContentsMargins(0, 0, 0, 0);
    this->toolBar->layout()->setContentsMargins(5, 0, 5, 0);

    this->bottomBar->setContentsMargins(0, 0, 0, 0);
    this->bottomBar->layout()->setContentsMargins(5, 0, 5, 0);

    this->hoveredAddressLabel = this->bottomBar->findChild<QLabel*>("byte-address-label");

    this->byteWidgetScrollArea = this->container->findChild<QScrollArea*>("byte-widget-scroll-area");
    auto byteWidgetScrollAreaWidgetContainer = this->byteWidgetScrollArea->findChild<QWidget*>(
        "byte-widget-scroll-area-container"
    );
    auto byteWidgetScrollAreaHorizontalLayout = byteWidgetScrollAreaWidgetContainer->findChild<QHBoxLayout*>(
        "byte-widget-scroll-area-horizontal-layout"
    );

    this->byteWidgetContainer = new ByteWidgetContainer(
        targetMemoryDescriptor,
        insightWorker,
        this->hoveredAddressLabel,
        byteWidgetScrollAreaHorizontalLayout->parentWidget()
    );

    byteWidgetScrollAreaHorizontalLayout->addWidget(this->byteWidgetContainer);

    this->byteWidgetAddressContainer = byteWidgetScrollAreaWidgetContainer->findChild<QWidget*>(
        "address-container"
    );
    this->byteWidgetAddressLayout = this->byteWidgetAddressContainer->findChild<QVBoxLayout*>();
    this->byteWidgetAddressLayout->setContentsMargins(5, 10, 0, 5);

    this->connect(
        this->byteWidgetContainer,
        &ByteWidgetContainer::byteWidgetsAdjusted,
        this,
        &HexViewerWidget::onByteWidgetsAdjusted
    );
    this->connect(
        &insightWorker,
        &InsightWorker::targetStateUpdated,
        this,
        &HexViewerWidget::onTargetStateChanged
    );

    this->show();
}

void HexViewerWidget::updateValues(const Targets::TargetMemoryBuffer& buffer) {
    this->byteWidgetContainer->updateValues(buffer);
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
    const auto& byteWidgetsByRowIndex = this->byteWidgetContainer->byteWidgetsByRowIndex;

    int layoutItemMaxIndex = this->byteWidgetAddressLayout->count() - 1;
    for (const auto& mappingPair : byteWidgetsByRowIndex) {
        const auto rowIndex = static_cast<int>(mappingPair.first);
        const auto& byteWidgets = mappingPair.second;

        if (byteWidgets.empty()) {
            continue;
        }

        QLabel* labelWidget;
        if (rowIndex > layoutItemMaxIndex) {
            labelWidget = new QLabel(this->byteWidgetAddressContainer);
            labelWidget->setFixedSize(75, 20);
            labelWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

            this->byteWidgetAddressLayout->addWidget(labelWidget);
            layoutItemMaxIndex++;

        } else {
            labelWidget = qobject_cast<QLabel*>(this->byteWidgetAddressLayout->itemAt(rowIndex)->widget());
        }

        labelWidget->setText(byteWidgets.front()->relativeAddressHex);
    }

    const auto rowCount = static_cast<int>(byteWidgetsByRowIndex.size());
    QLayoutItem* labelItem;
    while ((labelItem = this->byteWidgetAddressLayout->takeAt(rowCount)) != nullptr) {
        labelItem->widget()->deleteLater();
    }
}
